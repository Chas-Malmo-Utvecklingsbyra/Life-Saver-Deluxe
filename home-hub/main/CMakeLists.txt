file(GLOB_RECURSE CORE_SOURCES
    "../../embedded-core/*.c"
)

file(GLOB_RECURSE APP_SOURCES
    "*.c"
)

idf_component_register(
    SRCS ${APP_SOURCES} ${CORE_SOURCES}
    INCLUDE_DIRS "." "../../embedded-core"
    REQUIRES 
        spiffs
        esp_wifi
        nvs_flash
        esp_http_client
        lvgl
        esp_driver_i2c
        esp_driver_gpio
        esp_driver_uart
        esp_lcd
        esp_lcd_touch
        esp_lcd_touch_gt911
        esp_timer
        console
)