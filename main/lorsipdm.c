#include <stdio.h>
#include <esp_system.h>

#include "wifi_server.h"
#include "bluetooth_client.h"

#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/************************************************************/
/* Feature Enable/Disable Defines                           */
/************************************************************/
#define LORSI_NET  // Enables WiFi Client.
#define LORSI_BT // Enables BT Server.

/************************************************************/
/* Types Definitions                                        */
/************************************************************/

typedef void (*PDM_Runnable_t)();

typedef enum {
    PDM_NONE,
    PDM_WIFI,
    PDM_BT,
} PDM_DataSource_t;

typedef struct {
    PDM_DataSource_t source;
    uint32_t data;
} PDM_RequestEvent_t;

typedef enum {
    SLOW_BLINK = 0,
    FAST_BLINK,
    BT_DISABLED,
} PDM_State_t;

typedef struct {
    PDM_State_t currentState;
    PDM_RequestEvent_t event;
    PDM_State_t nextState;
    PDM_Runnable_t handler;
} PDM_FsmEntry_t; 

/************************************************************/
/* FSM State Variables                                      */
/************************************************************/
static PDM_State_t currentState_ = BT_DISABLED;
static bool isCachedEventLocked_ = false;
static bool isRequestPending_ = false;
static PDM_RequestEvent_t cachedEvent_ = {
    .source = PDM_NONE,
    .data = 0,
};

/************************************************************/
/* Event "Interruption" Subroutines                         */
/************************************************************/
static void PDM_WiFiDataHandler(uint32_t data) {
#ifdef LORSI_NET
    // If another event is being saved or enqueued, ignore this request.
    if(isRequestPending_ || isCachedEventLocked_)  { 
        return;
    }
    isCachedEventLocked_ = true;
    cachedEvent_.source = PDM_WIFI;
    cachedEvent_.data = data;
    isRequestPending_ = true;
    isCachedEventLocked_ = false;
#endif
}

static void PDM_BtDataHandler(uint32_t data) {
#ifdef LORSI_BT
    // If another event is being saved or enqueued, ignore this request.
    if(isRequestPending_ || isCachedEventLocked_)  { 
        return;
    }
    isCachedEventLocked_ = true;
    cachedEvent_.source = PDM_BT;
    cachedEvent_.data = data;
    isRequestPending_ = true;
    isCachedEventLocked_ = false;
#endif
}

/************************************************************/
/* FSM Handlers                                             */
/************************************************************/
static inline bool isBtEnabled() {
    return currentState_ != BT_DISABLED;
}

static inline uint32_t getBlinkingStatusCode() {
    return (uint32_t)currentState_; // Code matches state enum value.
}

static void sendCurrentBlinkSpeed() {
    PDMNetwork_send(getBlinkingStatusCode());
}

static void sendCurrentBTServiceStatus() {
    PDMNetwork_send(isBtEnabled() ? 0 : 1);
}

static void toggleAndSendBTServiceStatus() {
    PDMNetwork_send(isBtEnabled() ? 1 : 0);
}

static void setFastBlink() {
    // TODO
}

static void setSlowBlink() {
    // TODO
}

/************************************************************/
/* FSM Definition                                           */
/************************************************************/
static const PDM_FsmEntry_t fsmTable_[] = {
    {BT_DISABLED,  {PDM_WIFI, 0},    BT_DISABLED,  sendCurrentBlinkSpeed},
    {SLOW_BLINK,   {PDM_WIFI, 0},    SLOW_BLINK,   sendCurrentBlinkSpeed},
    {FAST_BLINK,   {PDM_WIFI, 0},    FAST_BLINK,   sendCurrentBlinkSpeed},

    {BT_DISABLED,  {PDM_WIFI, 1},    BT_DISABLED,  sendCurrentBTServiceStatus},
    {SLOW_BLINK,   {PDM_WIFI, 1},    SLOW_BLINK,   sendCurrentBTServiceStatus},
    {FAST_BLINK,   {PDM_WIFI, 1},    FAST_BLINK,   sendCurrentBTServiceStatus},

    {BT_DISABLED,  {PDM_WIFI, 2},    FAST_BLINK,    toggleAndSendBTServiceStatus},
    {SLOW_BLINK,   {PDM_WIFI, 2},    BT_DISABLED,    toggleAndSendBTServiceStatus},
    {FAST_BLINK,   {PDM_WIFI, 2},    BT_DISABLED,    toggleAndSendBTServiceStatus},

    {SLOW_BLINK,   {PDM_BT, 0},      FAST_BLINK,    setFastBlink},
    {FAST_BLINK,   {PDM_BT, 1},      SLOW_BLINK,    setSlowBlink},
};

/************************************************************/
/* FSM Methods                                              */
/************************************************************/
static void fsmSpin_() {
    for(BaseType_t i=0; i<sizeof(fsmTable_)/sizeof(PDM_FsmEntry_t); i++) {
        if(fsmTable_[i].currentState == currentState_ &&
           fsmTable_[i].event.source == cachedEvent_.source &&
           fsmTable_[i].event.data == cachedEvent_.data) {
               fsmTable_[i].handler();
               currentState_ = fsmTable_[i].nextState;
           }
    }
    isRequestPending_ = false;
}

/************************************************************/
/* Other Prv Methods                                        */
/************************************************************/
static void PDM_boardInit() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
}

static void init() {
    PDM_boardInit();
#ifdef LORSI_BT
    PDMBluetooth_init(PDM_BtDataHandler);
#endif
#ifdef LORSI_NET
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());
    while(!PDMNetwork_init(PDM_WiFiDataHandler)) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
#endif
}

static void loop() {
    #ifdef LORSI_NET
    PDMNetwork_task();
    #endif
    // PDMBluetooth_task(); -> Not needed, 'cause concurrency issues are fun (:
    fsmSpin_();
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
