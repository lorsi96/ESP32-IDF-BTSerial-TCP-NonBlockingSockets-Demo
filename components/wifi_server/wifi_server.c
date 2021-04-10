#include <stdio.h>
#include "wifi_server.h"

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


#define HOST_IP_ADDR "192.168.0.14"
#define PORT 3333

static const char *TAG = "wifi_server";
static IntConsumer_t onDataReceivedCallback;
static uint8_t reconnect = 0;


/** Internal Connection Constants. ****************/
static const char host_ip[] = HOST_IP_ADDR;
static const int addr_family = AF_INET;
static const int ip_protocol = IPPROTO_IP;

// static char tx_buffer[128];
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

    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));

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
        reconnect = 0;
        onDataReceivedCallback((uint32_t)(rx_buffer[0]-0x30));    
        rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
        ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
        return;
    }
}