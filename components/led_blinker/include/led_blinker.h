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
#ifndef __PDM_BLINK__
#define __PDM_BLINK__

#include <stdint.h>
#include "driver/gpio.h"


#define BLINK_GPIO GPIO_NUM_2
#define PDM_SLOW_SPEED_MS 1000
#define PDM_FAST_SPEED_MS 200

/**
 * @brief Speeds at which the led blinker can run.
 * 
 */
typedef enum {
    PDM_BLINK_SPEED_SLOW = 0,
    PDM_BLINK_SPEED_FAST = 1,
    PDM_BLINK_ALWAYS_ON = 2,
} PDM_BlinkSpeed_t;

/**
 * @brief Initializes the blink module.
 * 
 * @param blinkSpeed speed velocity according to PDM_BlinkSpeed_t enum.
 */
void PDMBlink_Init(const PDM_BlinkSpeed_t blinkSpeed);

/**
 * @brief Updates the blinkSpeed on the go.
 * 
 * @param blinkSpeed new speed.
 */
void PDMBlink_SpeedUpdate(const PDM_BlinkSpeed_t blinkSpeed);

/**
 * @brief Task to be run in the main loop of an application
 *        to keep the module going. 
 */
void PDMBlink_Task();

#endif // __PDM_BLINK__