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
/**
 * @brief TCP Sockets client that sends and receives uint32_t values from a 
 *         TCP Socket server.
*/
#ifndef _TCP_CLIENT_
#define _TCP_CLIENT_

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#define HOST_IP_ADDR "192.168.0.14"
#define PORT 3333

/** Syntatic sugar  for functions receiving an int and returning void.*/
typedef void(*IntConsumer_t)(const uint32_t value);

/**
 * @brief Initializes the PDM Network module.
 * 
 * @param onDataReceived function to be called when a message from the 
 *                        server arrives.
 * 
 * @return true if initalization succeeded.
 * @return false if there was an error when initializing the module.
 */
bool PDMNetwork_init(IntConsumer_t onDataReceived);

/**
 * @brief Sends a message to the server.
 * 
 * @param message to be sent.
 */
void PDMNetwork_send(const uint32_t message);

/**
 * @brief Task to be run in the main loop of an application
 *        to keep the module going. 
 * @note  This was only tested with a refresh rate of .5 seconds. 
 *        There are no guarantees that it will or will not work with
 *        higher or lower rates.
 */
void PDMNetwork_task();

#endif // _TCP_CLIENT_