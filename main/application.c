/* Copyright 2015-2016, lorsi96 (Lucas Orsi).
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <stdio.h>
#include <esp_system.h>

#include "tcp_client.h"
#include "bluetooth_client.h"

#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_blinker.h"

/************************************************************/
/* Feature Enable/Disable Defines                           */
/************************************************************/
#define LORSI_NET  /**< Enables WiFi Client.*/
#define LORSI_BT /**< Enables BT Server.*/

/************************************************************/
/* Type Definitions                                         */
/************************************************************/

typedef void (*PDM_Runnable_t)();

/**
 * @brief Source of an incoming event.
 * 
 * Events can be originated from either the TCP server or
 * the BT client. 
 */
typedef enum {
    PDM_NONE, /**< No source. Only used at the start of the program.*/
    PDM_WIFI, /**< Event from WiFi. Currently it identifies a message from the TCP server.*/
    PDM_BT,   /**< Event from BT Serial.*/
} PDM_DataSource_t;

/**
 * @brief Contains events captured by this application.
 */
typedef struct {
    PDM_DataSource_t source; /**< Source of the incoming event.*/
    uint32_t data; /**< Data received. Currently only supports uint32_t data.*/
} PDM_RequestEvent_t;

/**
 * @brief FSM states type.
 */
typedef enum {
    SLOW_BLINK = 0, /**< BuiltIn LED Blinking slowly.*/
    FAST_BLINK,  /**< BuiltIn LED Blinking fast.*/
    BT_DISABLED,  /**< BlueTooth events ignored - LED always on.*/
} PDM_State_t;

/**
 * @brief FSM Entry for the state table. 
 */
typedef struct {
    PDM_State_t currentState;   /**< State where the FSM is currently in.*/
    PDM_RequestEvent_t event;   /**< Relevant event for the current state.*/
    PDM_State_t nextState;      /**< State to move to after the event is processed.*/
    PDM_Runnable_t handler;     /**< Handler to be run when the event happens.*/
} PDM_FsmEntry_t; 

/************************************************************/
/* FSM State Variables                                      */
/************************************************************/
static PDM_State_t currentState_ = BT_DISABLED; /**< Current FSM state.*/
static bool isRequestPending_ = false; /**< Signals whether an event needs attention or not.*/
static PDM_RequestEvent_t cachedEvent_ = { /**< Cached event to be processed by the FSM.*/
    .source = PDM_NONE, 
    .data = 0,
};
static bool isCachedEventLocked_ = false; /**< Whether the cachedEvent is being locked by a handler or not.*/

/************************************************************/
/* Event "Interruption" Subroutines                         */
/************************************************************/
inline static void PDM_DataHandler_(const uint32_t data, const PDM_DataSource_t source) {
    if(isRequestPending_ || isCachedEventLocked_)  { 
        return; /** Return if there's already an event pending or if an event is being processed. */
    }
    isCachedEventLocked_ = true;
    cachedEvent_.source = source;
    cachedEvent_.data = data;
    isRequestPending_ = true;
    isCachedEventLocked_ = false;
}

static void PDM_WiFiDataHandler(uint32_t data) {
#ifdef LORSI_NET
    PDM_DataHandler_(data, PDM_WIFI);
#endif
}

static void PDM_BtDataHandler(uint32_t data) {
#ifdef LORSI_BT
    PDM_DataHandler_(data, PDM_BT);
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

static void updateBlink() {
    PDMBlink_SpeedUpdate((PDM_BlinkSpeed_t)currentState_); // Code matches state enum value.
}

static void doNothing() {}

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

    {BT_DISABLED,  {PDM_WIFI, 2},    FAST_BLINK,    sendCurrentBTServiceStatus},
    {SLOW_BLINK,   {PDM_WIFI, 2},    BT_DISABLED,   sendCurrentBTServiceStatus},
    {FAST_BLINK,   {PDM_WIFI, 2},    BT_DISABLED,   sendCurrentBTServiceStatus},

    {SLOW_BLINK,   {PDM_BT, 0},      FAST_BLINK,    doNothing},
    {FAST_BLINK,   {PDM_BT, 1},      SLOW_BLINK,    doNothing},
};

/************************************************************/
/* FSM Methods                                              */
/************************************************************/
static void fsmSpin_() {
    if(!isRequestPending_) {
        return;
    }
    for(BaseType_t i=0; i<sizeof(fsmTable_)/sizeof(PDM_FsmEntry_t); i++) {
        if(fsmTable_[i].currentState == currentState_ &&
           fsmTable_[i].event.source == cachedEvent_.source &&
           fsmTable_[i].event.data == cachedEvent_.data) {
               fsmTable_[i].handler();
               currentState_ = fsmTable_[i].nextState;
               updateBlink();
               break;
           }
    }
    isRequestPending_ = false;
}

/************************************************************/
/* Other Prv Methods                                        */
/************************************************************/

/**
 * @brief Initializes the ESP32 Board.
 */
static void PDM_boardInit() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
}

/**
 * @brief Initializes the ESP32 Board and the Wifi/BT/LED Modules.
 */
static void init() {
    PDM_boardInit();
    PDMBlink_Init(PDM_BLINK_ALWAYS_ON);
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

/**
 * @brief Loop function to keep the program updated and running.
 * 
 */
static void loop() {
    PDMBlink_Task();
    #ifdef LORSI_NET
    PDMNetwork_task();
    #endif
    #ifdef LORSI_BT
    PDMBluetooth_task();
    #endif
    fsmSpin_();
}

/**
 * @brief Task to imitate a Superloop architecture.
 *       
 * PDM Shouldn't require RTOS stuff directly.
 */
void superLoopTask(void* _) {
    init();
    for(;;) {
        loop();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    xTaskCreate(superLoopTask, "lorsi_pdm", 4096, NULL, 5, NULL);
}
