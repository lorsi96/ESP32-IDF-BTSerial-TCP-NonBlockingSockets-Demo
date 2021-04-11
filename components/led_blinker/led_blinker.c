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
#include "led_blinker.h"
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_log.h"

static const TickType_t blinkSpeedTableTicks[] = {
    pdMS_TO_TICKS(PDM_SLOW_SPEED_MS),
    pdMS_TO_TICKS(PDM_FAST_SPEED_MS)
};

static PDM_BlinkSpeed_t currentBlinkSpeed;
static TickType_t timeCount;
static bool ledOn = true;

void PDM_blinkInit(const PDM_BlinkSpeed_t blinkSpeed) {
  gpio_reset_pin(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(BLINK_GPIO, 1);        
  currentBlinkSpeed = blinkSpeed;
  timeCount = xTaskGetTickCount();
}

void PDM_blinkSpeedUpdate(const PDM_BlinkSpeed_t blinkSpeed) {
    currentBlinkSpeed = blinkSpeed;
    timeCount = xTaskGetTickCount();
    if(currentBlinkSpeed == PDM_BLINK_ALWAYS_ON) {
        ledOn = true;
        gpio_set_level(BLINK_GPIO, 1);        
    }
}

void PDM_blinkTask() {
    ESP_LOGI("BlinkTask", "Switch");
    switch(currentBlinkSpeed) {
    case PDM_BLINK_SPEED_SLOW ... PDM_BLINK_SPEED_FAST:
        ESP_LOGI("BlinkTask", "Speed Running %d", currentBlinkSpeed);
        if(xTaskGetTickCount() - timeCount > blinkSpeedTableTicks[currentBlinkSpeed]) {
            ledOn = !ledOn;
            gpio_set_level(BLINK_GPIO, ledOn);
            timeCount = xTaskGetTickCount();
        }
        break; 
    default:
        ESP_LOGI("BlinkTask", "Default");
        break;
    }
}
