#ifndef _BLE_CLIENT_
#define _BLE_BLIENT_

#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

typedef void(*IntConsumer_t)(const uint32_t value);

void PDMBluetooth_init(IntConsumer_t onDataReceived);

void PDMBluetooth_task();

#endif // _BLE_BLIENT_