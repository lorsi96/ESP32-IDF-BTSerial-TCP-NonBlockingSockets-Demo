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

#define EXAMPLE_DEVICE_NAME "ESP_SPP_ACCEPTOR"


typedef void(*IntConsumer_t)(const uint32_t value);

/**
 * @brief Initializes the Bluetooth module.
 * 
 * This module listens to Bluetooth Serial commands received from
 *  a Bluetooth Classic and forwards them to a handler.
 * 
 * @param onDataReceived handler that will consume captured messages.
 */
void PDMBluetooth_init(IntConsumer_t onDataReceived);

/** 
 * @brief Task to be run in the main loop of an application
 *        to keep the module going. 
 * @note Currently does nothing, but it's present so as to be
 *       consistent with the other modules of the PDM project.
*/
void PDMBluetooth_task();

/**
 * @brief Deinitializes the Bluetooth module.
 * 
 */
void PDMBluetooth_deinit(); /** TODO: Implement for future versions. */

#endif // _BLE_BLIENT_