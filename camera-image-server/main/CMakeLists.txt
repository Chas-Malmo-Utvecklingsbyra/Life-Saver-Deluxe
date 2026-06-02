idf_component_register(SRCS camera-image-server.c
                        camera.c
                        connect_wifi.c
                        INCLUDE_DIRS .
                        PRIV_INCLUDE_DIRS .
                        PRIV_REQUIRES nvs_flash esp_psram espressif__esp_jpeg esp_http_server esp_http_client esp_timer esp_wifi driver)
