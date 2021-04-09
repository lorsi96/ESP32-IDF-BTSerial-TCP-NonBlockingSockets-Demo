#include <stdio.h>
#include <esp_system.h>
#include "bluetooth_client.h"
#include "wifi_server.h"
#include "protocol_examples_common.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"

static void testConsumer(uint32_t data) {
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

static void init() {
    while(!PDMNetwork_init(testConsumer)) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void loop() {
    PDMNetwork_task();
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
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(superLoopTask, "lorsi_pdm", 4096, NULL, 5, NULL);
}
