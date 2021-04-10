#include <stdio.h>
#include <esp_system.h>
#include "wifi_server.h"
#include "protocol_examples_common.h"
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
#include "bluetooth_client.h"

#define LORSI_NET
#define LORSI_BT

#ifdef LORSI_NET
static void socketTestConsumer(uint32_t data) {
    switch(data) {
    case 0:
      ESP_LOGI("Main", "Speed request arrived");
      PDMNetwork_send(0);
      break;
    case 1:
      ESP_LOGI("Main", "Reset request arrived");
      break;
    }
}
#endif


#ifdef LORSI_BT
static void btTestConsumer(uint32_t data) {
    switch(data) {
    case 0:
      ESP_LOGI("Main", "Speed request arrived");
      break;
    case 1:
      ESP_LOGI("Main", "Reset request arrived");
      break;
    }
}
#endif

static void init() {
    #ifdef LORSI_BT
    PDMBluetooth_init(btTestConsumer);
    #endif
    #ifdef LORSI_NET
    
    ESP_ERROR_CHECK(esp_netif_init());
    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());
    while(!PDMNetwork_init(socketTestConsumer)) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    #endif
}

static void loop() {
    #ifdef LORSI_NET
    PDMNetwork_task();
    #endif
    
    // PDMBluetooth_task(); -> Not needed
}

void superLoopTask(void* pvParameters) {
    init();
    for(;;) {
        loop();
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{

    xTaskCreate(superLoopTask, "lorsi_pdm", 4096, NULL, 5, NULL);
}
