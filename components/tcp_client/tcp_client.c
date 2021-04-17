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
#include "tcp_client.h"

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"

static const char *TAG = "tcp_client";
static IntConsumer_t onDataReceivedCallback;

/** Internal Connection Constants. ****************/
static const char host_ip[] = HOST_IP_ADDR;
static const int addr_family = AF_INET;
static const int ip_protocol = IPPROTO_IP;

static char rx_buffer[128];
static struct sockaddr_in dest_addr;

static int sock;

/** Flow Variables ********************************/
static char dataToSend[] = "0";
static bool isDataToSendReady = false;

void static PDMNetwork_reinit() {
    ESP_LOGE(TAG, "Shutting down socket and restarting...");
    shutdown(sock, 0);
    close(sock);
    PDMNetwork_init(onDataReceivedCallback);
}

bool PDMNetwork_init(IntConsumer_t onDataReceived) {
    dest_addr.sin_addr.s_addr = inet_addr(host_ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    onDataReceivedCallback = onDataReceived;
    sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return false;
    }
    ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

    int err = connect(
        sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));

    if (fcntl(sock, F_SETFL,  O_NONBLOCK) < 0) {
        ESP_LOGE(TAG, "Failed to set nonblocking error");
    }

    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        return false;
    }
    ESP_LOGI(TAG, "Successfully connected");
    return true;
}

void PDMNetwork_send(const uint32_t message) {
    dataToSend[0] =  message + 0x30;
    isDataToSendReady = true;
}

void PDMNetwork_task() {
    ESP_LOGD(TAG, "Task Running");

    if (isDataToSendReady) {
        int err = send(sock, dataToSend, strlen(dataToSend), 0);
        ESP_LOGD(TAG, "Message sent");
        if (err < 0) {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            PDMNetwork_reinit();
        }
        isDataToSendReady = false;
    }

    ESP_LOGD(TAG, "Attempting Reception");
    int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, MSG_DONTWAIT);
    ESP_LOGD(TAG, "After Reception %d", len);
    if (len > 0) {
        onDataReceivedCallback((uint32_t)(rx_buffer[0]-0x30));    
        rx_buffer[len] = 0;
        ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
        return;
    }
}