#include "esp_bt.h"
#include "setting.h"
#include "esp_log.h"
#include <stdbool.h>
#include "nvs_flash.h"
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "driver/uart.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "nimble/nimble_port_freertos.h"

// * Just for testing
#include <stdio.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <esp_task_wdt.h>

// * Tag for logging
static const char *TAG = "uart_esp_server";

// Configure UART
#define UART UART_NUM_0 // USB UART
#define TX_PIN GPIO_NUM_16
#define RX_PIN GPIO_NUM_17
#define BUF_SIZE (2 * SOC_UART_FIFO_LEN)
#define QUEUE_SIZE 8
#define TX_MSG_LEN SBUFLEN
#define RX_MSG_LEN SBUFLEN

extern "C" void app_main(void)
{
    // Exclude the Idle Task from the Task WDT
    ESP_ERROR_CHECK(esp_task_wdt_delete(xTaskGetIdleTaskHandle()));

    // ! TO BE REMOVED
    ESP_ERROR_CHECK(gpio_reset_pin(GPIO_NUM_4));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT)); // Configure pin 4 as a digital output pin
    uint32_t led_state = 0;

    int64_t interval_us = Setting::INTERVAL;
    (void)interval_us;

    // ** For UART **
    QueueHandle_t queue;

    // Taken straight from docs, except for baud_rate:
    // - https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html
    uart_config_t config = {};
    config.baud_rate = BAUDRATE; // From setting.h
    config.data_bits = UART_DATA_8_BITS;
    config.parity = UART_PARITY_DISABLE;
    config.stop_bits = UART_STOP_BITS_1;
    config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    config.source_clk = UART_SCLK_DEFAULT;

    // Install driver and configure UART
    ESP_ERROR_CHECK(uart_driver_install(UART, BUF_SIZE, BUF_SIZE, QUEUE_SIZE, &queue, 0));
    ESP_ERROR_CHECK(uart_param_config(UART, &config));
    ESP_ERROR_CHECK(uart_set_pin(UART, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "UART initialized");

    // * To store received event
    uart_event_t event;
    uint8_t buffer[BUF_SIZE];

    while (1)
    {
        // Read UART
        if (pdTRUE == xQueueReceive(queue, (void *)&event, 1))
        {
            // Clear the buffer
            bzero(buffer, BUF_SIZE);

            // Handle events
            switch (event.type)
            {
            case UART_DATA:
            {
                int len = uart_read_bytes(UART, buffer, event.size, portMAX_DELAY);
                if (len == RX_MSG_LEN)
                {
                    // ! TO BE REMOVED
                    led_state = !led_state;
                    ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_4, led_state));

                    // TODO -> Send data over BLE
                }
                else
                {
                    ESP_LOGE(TAG, "Packet misaligned. Read %d bytes, expected %d", len, RX_MSG_LEN);
                }
                break;
            }

            case UART_FIFO_OVF: /* Event of HW FIFO overflow detected */
                ESP_LOGI(TAG, "Hardware FIFO overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                uart_flush_input(UART);
                xQueueReset(queue);
                break;

            case UART_BUFFER_FULL: /* Event of UART ring buffer full */
                ESP_LOGI(TAG, "Ring buffer full");
                // If buffer full happened, you should consider increasing your buffer size
                uart_flush_input(UART);
                xQueueReset(queue);
                break;

            case UART_BREAK: /* Event of UART RX break detected */
                ESP_LOGI(TAG, "UART rx break");
                break;

            case UART_PARITY_ERR: /* Event of UART parity check error */
                ESP_LOGI(TAG, "UART parity error");
                break;

            case UART_FRAME_ERR: /* Event of UART frame error */
                ESP_LOGI(TAG, "UART frame error");
                break;

            default: /* Others */
                ESP_LOGI(TAG, "UART event type: %d", event.type);
                break;
            }
        }
    }
}