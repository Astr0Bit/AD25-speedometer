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

void app_main(void)
{
}