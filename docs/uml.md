@startuml


ESP32S3 <- ESP32S3 : cJSON_InitHooks()
ESP32S3 <- ESP32S3 : Console_Initialize()
ESP32S3 <- ESP32S3 : lvgl_task()
ESP32S3 <- ESP32S3 : initialize_config()
ESP32S3 <- ESP32S3 : Internet_Initialize()
ESP32S3 <- ESP32S3 : tcp_server_task()
activate ESP32S3
alt Full_Data_Received == Packet_Job_Initialize
    ESP32S3 -> ESP32C3 : send_uuid()
else Full_Data_Received == Packet_Job_Data
    ESP32S3 -> ESP32S3 : update_sensor()
    ESP32S3 -> ESP32C3 : acknowledge_sensor()
    ESP32S3 --> ESP32C3 : Acknowledgement
end
deactivate ESP32S3

ESP32S3 <- ESP32S3 : mdns_init()
ESP32S3 <- ESP32S3 : bme280_task()


ESP32C3 -> ESP32C3 : initialize_uuid()
ESP32C3 -> ESP32C3 : Internet_Initialize()
ESP32C3 -> ESP32C3 : setup_gpio()

ESP32C3 -> ESP32C3 : sensor_read_task()
activate ESP32C3
loop Every 100 ms
    ESP32C3 -> ESP32C3 : gpio_get_level()
    ESP32C3 -> ESP32S3 : send_data_to_home_hub()
    ESP32C3 --> ESP32S3 : JSON data
    ESP32C3 <-- ESP32S3 : Received data acknowledgemenet
end
deactivate ESP32C3

ESP32C3 -> ESP32C3 : sensor_initialize_task()
ESP32C3 -> ESP32C3 : read_tcp_task()


@enduml