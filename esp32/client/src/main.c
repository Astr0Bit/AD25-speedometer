#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "services/gap/ble_svc_gap.h"
#include "nimble/nimble_port_freertos.h"
#include "setting.h"

#define TAG "BLE-UART"

#define UART UART_NUM_0
#define UART_RX_BUF_SIZE (2 * SOC_UART_FIFO_LEN)
#define UART_TX_BUF_SIZE ((2 * SBUFLEN >= UART_RX_BUF_SIZE) ? 2 * SBUFLEN : UART_RX_BUF_SIZE)

#define DEVICE_NAME TAG

static const uint8_t s_server_addr[] = SERVER_ADDR;
static const uint8_t s_client_addr[] = CLIENT_ADDR;

/*cb930000-0fbf-442d-b2f6-9e9338c4a27a*/
static const ble_uuid128_t s_gatt_svc_uuid = BLE_UUID128_INIT(GATT_SVC_UUID);
/*cb930001-0fbf-442d-b2f6-9e9338c4a27a*/
static const ble_uuid128_t s_gatt_chr_uuid = BLE_UUID128_INIT(GATT_CHR_UUID);

static uint8_t s_addr_type;
static ble_addr_t s_peer_addr;
static uint16_t s_svc_end_handle;
static uint16_t s_chr_val_handle;

static bool s_enc_done = false;
static bool s_chr_found = false;

extern void ble_store_config_init(void);
static int gap_event(struct ble_gap_event* event, void* arg);
static void try_start_dsc_discovery(uint16_t conn_handle);

static void scan(void)
{
    struct ble_gap_disc_params disc_params = {0};

    disc_params.passive = 1;           /* Perform a passive scan. */
    disc_params.filter_duplicates = 1; /* Avoid processing repeated advertisements from the same device. */

    int status = ble_gap_disc(s_addr_type, BLE_HS_FOREVER, &disc_params, gap_event, NULL);

    if (status != 0)
    {
        ESP_LOGE(TAG, "Error initiating GAP discovery procedure; rc=%d\n", status);
    }
}

static int on_subscription(uint16_t conn_handle, const struct ble_gatt_error* error, struct ble_gatt_attr* attr, void*)
{
    if (error->status != 0)
    {
        ble_gap_terminate(conn_handle, BLE_ERR_CONN_TERM_LOCAL);
    }
    return error->status;
}
static int on_descriptor_discovery(uint16_t conn_handle, const struct ble_gatt_error* error, uint16_t chr_val_handle, const struct ble_gatt_dsc* dsc, void*)
{
    if ((error->status == 0) && (dsc != NULL))
    {
        if (0 == ble_uuid_cmp(&dsc->uuid.u, BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16)))
        {
            /* Subscribe to notifications for the characteristic.
             * A central enables notifications by writing two bytes (0x01 00) to the
             * characteristic's client-characteristic-configuration-descriptor (CCCD).
             * Notification: 0x01 00, Indication: 0x02 00 and Disable both: 0x00 00
             */
            uint8_t value[2] = {1, 0};
            int rc = ble_gattc_write_flat(conn_handle, dsc->handle, value, sizeof(value), on_subscription, NULL);
            if (rc == 0)
            {
                return 1;
            }
            else
            {
                ESP_LOGE(TAG, "Error: Writing characteristic value failed; rc=0x%03x", rc);
                ble_gap_terminate(conn_handle, BLE_ERR_CONN_TERM_LOCAL);
            }
        }
    }
    else if (error->status != BLE_HS_EDONE)
    {
        ESP_LOGE(TAG, "Descriptor discovery failed: %d", error->status);
        ble_gap_terminate(conn_handle, BLE_ERR_CONN_TERM_LOCAL);
    }

    return 0;
}
static int on_characteristic_discovery(uint16_t conn_handle, const struct ble_gatt_error* error, const struct ble_gatt_chr* chr, void*)
{
    if ((error->status == 0) && (chr != NULL))
    {
        s_chr_val_handle = chr->val_handle;
        s_chr_found = true;
        try_start_dsc_discovery(conn_handle);
    }
    else if (error->status != BLE_HS_EDONE)
    {
        ESP_LOGE(TAG, "Characteristic discovery error: %d", error->status);
        ble_gap_terminate(conn_handle, BLE_ERR_CONN_TERM_LOCAL);
    }

    return 0;
}
static int on_service_discovery(uint16_t conn_handle, const struct ble_gatt_error* error, const struct ble_gatt_svc* service, void*)
{
    if ((error->status == 0) && (service != NULL))
    {
        s_svc_end_handle = service->end_handle;
        int rc = ble_gattc_disc_chrs_by_uuid(conn_handle, service->start_handle, service->end_handle, &s_gatt_chr_uuid.u, on_characteristic_discovery, NULL);
        if (rc != 0)
        {
            ESP_LOGE(TAG, "Error: Characteristics discovery failed; rc=0x%03x", rc);
            ble_gap_terminate(conn_handle, BLE_ERR_CONN_TERM_LOCAL);
        }
    }
    else if (error->status != BLE_HS_EDONE)
    {
        ESP_LOGE(TAG, "Service discovery failed; status=%d\n", error->status);
        ble_gap_terminate(conn_handle, BLE_ERR_CONN_TERM_LOCAL);
    }

    return 0;
}
static void on_reset(int reason)
{
    ESP_LOGE(TAG, "Resetting state; reason=%d\n", reason);
}
static void on_sync(void)
{
    int rc = ble_hs_id_set_rnd(s_client_addr);
    assert(rc == 0); // Set random static address; BLE_ADDR_RANDOM

    /* Make sure we have proper identity address set (public preferred) */
    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Figure out address to use while advertising */
    rc = ble_hs_id_infer_auto(0, &s_addr_type);
    assert(rc == 0);

    uint8_t addr[sizeof(s_client_addr)] = {0};
    rc = ble_hs_id_copy_addr(s_addr_type, addr, NULL);
    assert(rc == 0);

    ESP_LOGI(TAG, "BLE Device Address: %02X:%02X:%02X:%02X:%02X:%02X\n", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);

    /* Begin scanning for a peripheral to connect to. */
    scan();
}

static void try_start_dsc_discovery(uint16_t conn_handle)
{
    if (s_enc_done && s_chr_found)
    {
        int rc = ble_gattc_disc_all_dscs(conn_handle, s_chr_val_handle, s_svc_end_handle, on_descriptor_discovery, NULL);
        if (rc != 0) { ble_gap_terminate(conn_handle, BLE_ERR_CONN_TERM_LOCAL); }
    }
}

static bool should_connect(const struct ble_gap_disc_desc* disc)
{
    bool status = false;
    struct ble_hs_adv_fields fields;
    if (0 == ble_hs_adv_parse_fields(&fields, disc->data, disc->length_data))
    {
        if (0 == memcmp(disc->addr.val, s_server_addr, sizeof(s_server_addr)) &&
            0 != memcmp(disc->addr.val, s_peer_addr.val, sizeof(s_peer_addr.val)))
        {
            if ((disc->event_type == BLE_HCI_ADV_RPT_EVTYPE_ADV_IND) ||
                (disc->event_type == BLE_HCI_ADV_RPT_EVTYPE_DIR_IND))
            {
                for (uint8_t i = 0; i < fields.num_uuids128; ++i)
                {
                    if (0 == ble_uuid_cmp(&fields.uuids128[i].u, &s_gatt_svc_uuid.u))
                    {
                        status = true;
                        break;
                    }
                }
            }
        }
    }
    return status;
}
static void connect_if_interesting(const struct ble_gap_disc_desc* disc)
{
    if (should_connect(disc))
    {
        /* Scanning must be stopped before a connection can be initiated. */
        int status = ble_gap_disc_cancel();

        if (status == 0)
        {
            /* Try to connect the advertiser. 30 seconds timeout. It can be BLE_HS_FOREVER */
            status = ble_gap_connect(s_addr_type, &disc->addr, 30000, NULL, gap_event, NULL);

            if (status != 0)
            {
                char addr_str[18] = {0};
                sprintf(addr_str, "%02X:%02X:%02X:%02X:%02X:%02X",
                        disc->addr.val[5], disc->addr.val[4], disc->addr.val[3],
                        disc->addr.val[2], disc->addr.val[1], disc->addr.val[0]);
                ESP_LOGE(TAG, "Error: Failed to connect to device; addr_type=%d addr=%s; status=%d\n", disc->addr.type, addr_str, status);
            }
        }
        else
        {
            ESP_LOGE(TAG, "Failed to cancel scan; status=%d\n", status);
        }
    }
}

static int gap_event(struct ble_gap_event* event, void*)
{
    int status = 0;
    int rc = 0;
    struct ble_gap_conn_desc desc;

    switch (event->type)
    {
    case BLE_GAP_EVENT_DISC:
        connect_if_interesting(&event->disc);
        break;

    case BLE_GAP_EVENT_CONNECT: /* A new connection was established or a connection attempt failed. */
        if (event->connect.status == 0)
        {
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            if (rc != 0)
            {
                ESP_LOGE(TAG, "Error: Searching for connection descriptor failed; rc=0x%03x", rc);
                ble_gap_terminate(event->connect.conn_handle, BLE_ERR_CONN_TERM_LOCAL);
                break;
            }

            rc = ble_gap_security_initiate(event->connect.conn_handle); /* Request connection encryption */
            if (rc != 0)
            {
                ESP_LOGE(TAG, "Error: Security initiate failed; rc=0x%03x", rc);
                ble_gap_terminate(event->connect.conn_handle, BLE_ERR_CONN_TERM_LOCAL);
                break;
            }
            memcpy(s_peer_addr.val, desc.peer_id_addr.val, sizeof(desc.peer_id_addr.val));

            rc = ble_gattc_disc_svc_by_uuid(event->connect.conn_handle, &s_gatt_svc_uuid.u, on_service_discovery, NULL);
            if (rc != 0)
            {
                ESP_LOGE(TAG, "Error: Service discovery failed; rc=0x%03x", rc);
                ble_gap_terminate(event->connect.conn_handle, BLE_ERR_CONN_TERM_LOCAL);
                break;
            }
        }
        else
        {
            /* Connection attempt failed; resume scanning. */
            ESP_LOGE(TAG, "Error: Connection failed; status=%d\n", event->connect.status);
            scan();
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        memset(s_peer_addr.val, 0, sizeof(s_peer_addr.val)); /* Forget about the peer */
        s_chr_found = false;
        s_enc_done = false;
        scan();                                          /* Resume scanning */
        break;

    case BLE_GAP_EVENT_ENC_CHANGE: /* Encryption change event: Encryption has been enabled or disabled for this connection. */
        if (event->enc_change.status == 0)
        {
            s_enc_done = true;
            try_start_dsc_discovery(event->enc_change.conn_handle);
        }
        else
        {
            ESP_LOGE(TAG, "connection encryption failed, status: %d", event->enc_change.status);
        }
        break;

    case BLE_GAP_EVENT_NOTIFY_RX:
    {
        const uint16_t pktlen = OS_MBUF_PKTLEN(event->notify_rx.om);
        char buffer[SBUFLEN];
        if (pktlen == SBUFLEN)
        {
            rc = os_mbuf_copydata(event->notify_rx.om, 0, SBUFLEN, buffer);
            assert(rc == 0);
            uart_write_bytes(UART, buffer, SBUFLEN);
        }
        else
        {
            ESP_LOGE(TAG, "Error: Received %u bytes, Expected: %u", pktlen, SBUFLEN);
        }
        break;
    }
    case BLE_GAP_EVENT_REPEAT_PAIRING:
    {
        if (0 == ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc))
        {
            ESP_LOGW(TAG, "Stale bond for peer, deleting and re-pairing");
            rc = ble_store_util_delete_peer(&desc.peer_id_addr);
            assert(rc == 0);
            status = BLE_GAP_REPEAT_PAIRING_RETRY;
        }
        else
        {
            status = BLE_GAP_REPEAT_PAIRING_IGNORE;
        }
        break;
    }
    default:
        break;
    }

    return status;
}

void app_main(void)
{
    uart_config_t config = {
        .baud_rate = BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART, &config));

    esp_err_t status = nvs_flash_init();
    if (status == ESP_ERR_NVS_NO_FREE_PAGES || status == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        status = nvs_flash_init();
    }
    ESP_ERROR_CHECK(status);

    ESP_ERROR_CHECK(nimble_port_init());

    /* Configure the host. */
    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Security manager configuration */
    ble_hs_cfg.sm_mitm = 1;    // MITM protection = required for passkey
    ble_hs_cfg.sm_sc = 1;      // Secure Connections
    ble_hs_cfg.sm_bonding = 1; // Enable bonding
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_KEYBOARD_ONLY;
    ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC;
    ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC;
    ble_store_config_init();

    int rc = ble_sm_configure_static_passkey(PASSKEY, true);
    assert(rc == 0);

    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set(DEVICE_NAME);
    assert(rc == 0);

    nimble_port_run();
    nimble_port_freertos_deinit();
}