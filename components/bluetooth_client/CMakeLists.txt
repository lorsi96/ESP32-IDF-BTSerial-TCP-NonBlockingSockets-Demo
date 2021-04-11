cmake_minimum_required(VERSION 3.5)
idf_component_register(SRCS "bluetooth_client.c"
                    INCLUDE_DIRS "include"
                    REQUIRES bt freertos nvs_flash)
