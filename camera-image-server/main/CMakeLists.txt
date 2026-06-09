file(GLOB_RECURSE APP_SOURCES
    "*.c"
)

set(EMBEDDED_CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../embedded-core")

idf_component_register(
    SRCS ${APP_SOURCES} "../../embedded-core/json/cJSON.c"
    INCLUDE_DIRS "." "${EMBEDDED_CORE_DIR}"
    REQUIRES
        nvs_flash 
        esp_psram 
        espressif__esp_jpeg 
        esp_http_server 
        esp_http_client 
        esp_timer 
        esp_wifi 
        driver 
        esp_driver_gpio
        console
)
