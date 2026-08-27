#include "esp_bt.h"
#include "setting.h"
#include "esp_log.h"
#include <stdbool.h>
#include "led_strip.h"
#include <esp_timer.h>
#include "nvs_flash.h"
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/ble_sm.h"
#include "driver/uart.h"
#include <esp_task_wdt.h>
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "nimble/nimble_port_freertos.h"

// * Tag for logging
static const char *TAG = "SERVER";

// Custom type for BLE error codes
typedef enum
{
    BLE_OK = 0,
    BLE_ERR_GATT_SVR_INIT,
    BLE_ERR_BLE_SVC_GAP_DEV_NAME,
} ble_err_t;

// Configure UART
#define UART UART_NUM_0 // USB UART
#define TX_PIN GPIO_NUM_16
#define RX_PIN GPIO_NUM_17
#define RX_MSG_LEN SBUFLEN
#define UART_TIMEOUT_MS 100
#define BLE_DISCONNECT_INTERVAL_MS 500
#define UART_BUF_SIZE (2 * SOC_UART_FIFO_LEN)

// Configure RGB LED
#define RGB_PIN GPIO_NUM_8
#define LED_HSV_OFF 0, 0, 0
#define LED_HSV_PKT 210, 255, 10
#define LED_HSV_INIT 37, 255, 10
#define LED_HSV_CONNECTED 120, 255, 10
static led_strip_handle_t s_led;

// Configure BLE
#define NO_CONN_HANDLE 0xFFFF // When there is no active connection handle

// == Function declarations ==
void configure_led(void);

// BLE error codes using onboard RGB led
void ble_err_strobe();
void ble_err_success();
void ble_err_heartbeat();

// For BLE
static void on_sync(void);
static void advertise(void);
static ble_err_t ble_setup();
static int gatt_svr_init(void);
static void on_reset(int reason);
extern void ble_store_config_init(void);
static int gap_event(struct ble_gap_event *event, void *arg);
static void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *);
static void set_rgb(led_strip_handle_t led, uint16_t hue, uint8_t saturation, uint8_t value);
static int service_gatt_handler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) { return 0; }

static uint8_t addr_type;
static uint16_t chrval_handle;
static uint16_t active_conn_handle = NO_CONN_HANDLE; // Accessed outside the notify function

// For random static address, 2 MSB bits of the first byte shall be 0b11.
// I.e. addr[5] shall be in the range of 0xC0 to 0xFF
static const uint8_t server_addr[] = SERVER_ADDR;
static const uint8_t client_addr[] = CLIENT_ADDR;

// Services
static const ble_uuid128_t svc_uuid = BLE_UUID128_INIT(GATT_SVC_UUID);
static const ble_uuid128_t chr_uuid = BLE_UUID128_INIT(GATT_CHR_UUID);
static const struct ble_gatt_svc_def ble_svc_gatt_defs[] = {
    {
        /* The Service */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &svc_uuid.u,
        .includes = NULL,
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                /* The characteristic */
                .uuid = &chr_uuid.u,
                .access_cb = service_gatt_handler,
                .arg = NULL,
                .descriptors = NULL,
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC | BLE_GATT_CHR_F_WRITE_AUTHEN |
                         BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_NOTIFY_INDICATE_ENC | BLE_GATT_CHR_F_NOTIFY_INDICATE_AUTHEN,
                .min_key_size = 0,
                .val_handle = &chrval_handle,
                .cpfd = NULL,
            },
            {
                /* No more characteristics */
            },
        },
    },
    {
        /* No more services. */
    },
};

// Task to make BLE non-blocking
static void ble_host_task(void *param)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void app_main(void)
{
    // Exclude the Idle Task from the Task WDT
    ESP_ERROR_CHECK(esp_task_wdt_delete(xTaskGetIdleTaskHandle()));

    configure_led();

    // ** For UART **
    // Taken straight from docs, except for baud_rate:
    // - https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html
    uart_config_t config = {
        .baud_rate = BAUDRATE, // From setting.h
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Install driver and configure UART
    ESP_ERROR_CHECK(uart_driver_install(UART, UART_BUF_SIZE, UART_BUF_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART, &config));
    ESP_ERROR_CHECK(uart_set_pin(UART, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // * To store received event
    uint8_t buffer[SBUFLEN];

    // ** For BLE **
    ble_err_t ble_status = ble_setup();
    // ble_status = BLE_ERR_GATT_SVR_INIT; // Just for testing different error codes
    bool run_seq = true;
    while (run_seq)
    {
        // Blink LED in an error sequence
        switch (ble_status)
        {
        case BLE_ERR_GATT_SVR_INIT:
            ble_err_strobe();
            break;

        case BLE_ERR_BLE_SVC_GAP_DEV_NAME:
            ble_err_heartbeat();
            break;

        case BLE_OK:
            ble_err_success();
            run_seq = false; // Exit loop
            break;
        }
    }

    // Start the BLE task as a separate non-blocking task
    nimble_port_freertos_init(ble_host_task);

    while (true)
    {
        // Whilst connected to BLE client
        while (active_conn_handle != NO_CONN_HANDLE)
        {
            // Clear the buffer
            bzero(buffer, SBUFLEN);

            // Read UART
            int len = uart_read_bytes(UART, buffer, SBUFLEN, UART_TIMEOUT_MS);
            if (len == RX_MSG_LEN)
            {
                struct os_mbuf *txom = ble_hs_mbuf_from_flat(buffer, sizeof(buffer));
                if (0 == ble_gatts_notify_custom(active_conn_handle, chrval_handle, txom))
                {
                    set_rgb(s_led, LED_HSV_PKT);
                }
                else
                {
                    ESP_LOGE(TAG, "Error in sending notification");
                }
            }
            else if (len > 0)
            {
                ESP_LOGE(TAG, "Packet misaligned. Read %d bytes, expected %d", len, RX_MSG_LEN);
            }
            else if (len == 0)
            {
                // Nothing to read
                set_rgb(s_led, LED_HSV_CONNECTED);
            }
            else
            {
                ESP_LOGE(TAG, "Error in reading UART");
            }

            // Clear buffer after sending packet
            uart_flush_input(UART);
        }

        // Clear buffer when not connected to BLE client
        uart_flush_input(UART);

        // Turn of LED if not connected to BLE client
        set_rgb(s_led, LED_HSV_OFF);

        // Delay to not break the ESP32
        vTaskDelay(pdMS_TO_TICKS(BLE_DISCONNECT_INTERVAL_MS));
    }
}

// == Function implementations ==
// Helper for setting the RGB led
static void set_rgb(led_strip_handle_t led, uint16_t hue, uint8_t saturation, uint8_t value)
{
    ESP_ERROR_CHECK(led_strip_set_pixel_hsv(s_led, 0, hue, saturation, value));
    ESP_ERROR_CHECK(led_strip_refresh(s_led));
}

// Blinking patterns for BLE error codes
void ble_err_strobe()
{
    static const int N_STROBES = 6;
    static const int STROBE_SHORT_MS = 50;
    static const int STROBE_LONG_MS = 150;
    static const int STROBE_PAUSE_MS = 800;

    for (int i = 0; i < N_STROBES; i++)
    {
        set_rgb(s_led, LED_HSV_INIT);
        vTaskDelay(pdMS_TO_TICKS(STROBE_SHORT_MS));
        set_rgb(s_led, LED_HSV_OFF);
        vTaskDelay(pdMS_TO_TICKS(STROBE_LONG_MS));
    }
    vTaskDelay(pdMS_TO_TICKS(STROBE_PAUSE_MS));
}

void ble_err_heartbeat()
{
    static const int BEAT_MS = 100;
    static const int BEAT_PAUSE_MS = 700;

    set_rgb(s_led, LED_HSV_INIT);
    vTaskDelay(pdMS_TO_TICKS(BEAT_MS));
    set_rgb(s_led, LED_HSV_OFF);
    vTaskDelay(pdMS_TO_TICKS(BEAT_MS));
    set_rgb(s_led, LED_HSV_INIT);
    vTaskDelay(pdMS_TO_TICKS(BEAT_MS));

    set_rgb(s_led, LED_HSV_OFF);
    vTaskDelay(pdMS_TO_TICKS(BEAT_PAUSE_MS)); // Pause before repeating sequence
}

void ble_err_success()
{
    static const int SUCCESS_PAUSE_MS = 2000;
    set_rgb(s_led, LED_HSV_INIT);
    vTaskDelay(pdMS_TO_TICKS(SUCCESS_PAUSE_MS));
    set_rgb(s_led, LED_HSV_OFF);
}

// Configure the onboard RGB led
void configure_led(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_PIN,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {
            .invert_out = false,
        }};
    led_strip_spi_config_t spi_config = {
        .clk_src = SPI_CLK_SRC_DEFAULT,
        .spi_bus = SPI2_HOST,
        .flags = {
            .with_dma = true,
        }};
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &s_led));
    set_rgb(s_led, LED_HSV_INIT);
}

// For BLE
static void advertise(void)
{
    struct ble_hs_adv_fields fields = {0};
    struct ble_hs_adv_fields rsp_fields = {0};
    const char *name = ble_svc_gap_device_name();

    // General discoverability and BLE-only (BR/EDR unsupported)
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    /* 128-bit service UUIDs (alert notifications) */
    fields.uuids128 = &svc_uuid;
    fields.uuids128_is_complete = 1;
    fields.num_uuids128 = 1;

    // Try to set advertising fields
    int status = ble_gap_adv_set_fields(&fields);
    if (status != 0)
    {
        ESP_LOGE(TAG, "Error setting primary advertisement data; status = %d\n", status);
        return;
    }

    // Scan response data
    /* Set device name */
    rsp_fields.name = (uint8_t *)name;
    rsp_fields.name_len = strlen(name);
    rsp_fields.name_is_complete = 1;

    /* Set device tx power */
    rsp_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    rsp_fields.tx_pwr_lvl_is_present = 1;

    /* Set device LE role */
    rsp_fields.le_role = BLE_GAP_ROLE_SLAVE;
    rsp_fields.le_role_is_present = 1;

    // Try to set response fields
    status = ble_gap_adv_rsp_set_fields(&rsp_fields);
    if (status != 0)
    {
        ESP_LOGE(TAG, "Error setting scan response data; status = %d\n", status);
        return;
    }

    // Start advertising
    struct ble_gap_adv_params adv_params = {0};

    /* Set connetable and general discoverable mode */
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    adv_params.filter_policy = BLE_HCI_ADV_FILT_BOTH;

    /* Start advertising */
    status = ble_gap_adv_start(addr_type, NULL, BLE_HS_FOREVER, &adv_params, gap_event, NULL);
    if (status != 0)
    {
        ESP_LOGE(TAG, "Failed to start advertising, error code: %d", status);
    }
}

static int gap_event(struct ble_gap_event *event, void *)
{
    int status = 0;
    struct ble_gap_conn_desc desc;

    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT: /* A new connection was established or a connection attempt failed. */
        if (event->connect.status == 0)
        {
            assert(0 == ble_gap_conn_find(event->connect.conn_handle, &desc));
        }
        else
        {
            /* Connection failed; resume advertising. */
            advertise();
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        // Clear handle on disconnect
        active_conn_handle = NO_CONN_HANDLE;
        advertise(); /* Connection terminated; resume advertising. */
        break;

    case BLE_GAP_EVENT_CONN_UPDATE: /* The central has updated the connection parameters. */
        assert(0 == ble_gap_conn_find(event->conn_update.conn_handle, &desc));
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        advertise();
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        // Save current connection handle
        active_conn_handle = event->subscribe.conn_handle;

        // Set color of on-board RGB led
        set_rgb(s_led, LED_HSV_CONNECTED);
        break;

    case BLE_GAP_EVENT_REPEAT_PAIRING:                                            /* Repeat pairing event */
        assert(0 == ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc)); /* Get connection descriptor */
        assert(0 == ble_store_util_delete_peer(&desc.peer_id_addr));              /* Delete the old bond */

        status = BLE_GAP_REPEAT_PAIRING_RETRY; // Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that
                                               // the host should continue with pairing operation
        break;

    case BLE_GAP_EVENT_ENC_CHANGE: /* Encryption change event: Encryption has been enabled or disabled for this connection. */
        if (event->enc_change.status != 0)
        {
            ESP_LOGE(TAG, "connection encryption failed, status: %d", event->enc_change.status);
        }
        break;

    case BLE_GAP_EVENT_PASSKEY_ACTION:
    {
        struct ble_sm_io pkey = {0};
        pkey.action = event->passkey.params.action;

        // If the central expects us to display a passkey, we inject our static one
        if (pkey.action == BLE_SM_IOACT_DISP)
        {
            pkey.passkey = PASSKEY;
            ble_sm_inject_io(event->passkey.conn_handle, &pkey);
        }
        break;
    }

    default:
        break;
    }

    return status;
}

static void on_reset(int reason)
{
    ESP_LOGE(TAG, "Resetting state; reason=%d\n", reason);
}

static void on_sync(void)
{
    assert(0 == ble_hs_id_set_rnd(server_addr)); // Set random static address; BLE_ADDR_RANDOM

    assert(0 == ble_hs_util_ensure_addr(0));

    /* Figure out address type to use while advertising */
    assert(0 == ble_hs_id_infer_auto(0, &addr_type));

    uint8_t addr_val[6] = {0};
    assert(0 == ble_hs_id_copy_addr(addr_type, addr_val, NULL));

    printf("BLE Device Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
           addr_val[5], addr_val[4], addr_val[3], addr_val[2], addr_val[1], addr_val[0]);

    ble_addr_t client = {0};
    client.type = BLE_ADDR_RANDOM;
    memcpy(client.val, client_addr, sizeof(client_addr));

    assert(0 == ble_gap_wl_set(&client, 1));

    /* Begin advertising. */
    advertise();
}

static void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *)
{
    switch (ctxt->op)
    {
    case BLE_GATT_REGISTER_OP_SVC:
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        break;

    default:
        assert(0);
        break;
    }
}

static int gatt_svr_init(void)
{
    ble_svc_gap_init();
    ble_svc_gatt_init();

    int status = ble_gatts_count_cfg(ble_svc_gatt_defs);

    if (status == 0)
    {
        status = ble_gatts_add_svcs(ble_svc_gatt_defs);
    }

    return status;
}

static ble_err_t ble_setup()
{
    esp_err_t status = nvs_flash_init();
    if (status == ESP_ERR_NVS_NO_FREE_PAGES || status == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        status = nvs_flash_init();
    }
    ESP_ERROR_CHECK(status);

    ESP_ERROR_CHECK(nimble_port_init());
    ESP_ERROR_CHECK(esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P20));

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Security manager configuration */
    ble_hs_cfg.sm_sc = 1;      // Secure Connections
    ble_hs_cfg.sm_mitm = 1;    // MITM protection = required for passkey
    ble_hs_cfg.sm_bonding = 1; // Enable bonding
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_DISPLAY_ONLY;
    ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC;
    ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC;
    ble_store_config_init();

    /* Register custom service */
    if (0 != gatt_svr_init())
    {
        return BLE_ERR_GATT_SVR_INIT;
    }

    /* Set the default device name. */
    if (0 != ble_svc_gap_device_name_set(TAG))
    {
        return BLE_ERR_BLE_SVC_GAP_DEV_NAME;
    }

    return BLE_OK;
}