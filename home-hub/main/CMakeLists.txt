file(GLOB_RECURSE CORE_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/../../embedded-core/*.c"
)

idf_component_register(
    SRCS "home-hub.c" "sensor/sensor.c" ${CORE_SOURCES}
    INCLUDE_DIRS "." "../../embedded-core"
    REQUIRES 
        spiffs
        esp_wifi
        nvs_flash
        esp_http_client
        lvgl
        esp_driver_i2c
        esp_driver_gpio
        esp_lcd
        esp_lcd_touch
        esp_lcd_touch_gt911
        esp_timer
)