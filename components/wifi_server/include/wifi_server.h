#ifndef _WIFI_SERVER_
#define _WIFI_SERVER_

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

typedef void(*IntConsumer_t)(const uint32_t value);


bool PDMNetwork_init(IntConsumer_t onDataReceived);

void PDMNetwork_send(const uint32_t message);

void PDMNetwork_task();

// void tcp_client_task(void *pvParameters);

#endif // _WIFI_SERVER_