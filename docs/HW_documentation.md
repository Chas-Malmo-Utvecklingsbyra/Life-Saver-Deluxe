System overview:

The purpose of the system is to provide a modular home monitoring solution
consisting of distributed magnetic sensors, environmental monitoring and
camera surveillance.
  
The system includes:
* Magnetic sensors for doors and windows connected to ESP32-C3 sensor nodes
* Temperature, pressure and humidity measurements using a BME280 sensor
* Wireless communication betweeen the Home Hub and sensor nodes
* Camera-based monitoring using an ESP32-CAM module

+----------------+      +----------------+
|    ESP32-S3    |_I2C__|    BME280      |
|  Central Unit  |      | Temp/RH/Press  |
+-------+--------+      +-------+--------+
        |
        | Wi-Fi
        |
+-------+--------+       +----------------+
|   ESP32-C3 #1  | ----> | Magnetic Sensor|
+----------------+       +----------------+

+----------------+       +----------------+
|   ESP32-C3 #2  | ----> | Magnetic Sensor|
+----------------+       +----------------+

+----------------+       +----------------+
|   ESP32-C3 #3  | ----> | Magnetic Sensor|
+----------------+       +----------------+

  // Still in an experimental phase and not fully integrated 
+----------------+      +----------------+
|   ESP32-CAM    |______|     Motion     |
|  Camera Module |      |   Detection    |
+----------------+      +----------------+

  
Hardware components:
1. ESP32-S3
  The systems central unit which is responsible for:
  * GUI (LVGL)
  * Communication with sensor nodes
  * Handling of sensor-placement
  * Data collecting from BME280-sensor
Technical data:
  Parameter                  Value
  ---------                  ------
  Processor                  Xtensa LX7 Dual-Core
  Flash                      8 MB
  RAM                        512 kb
  Communication              Wi-Fi, Bluetooth
  Voltage                    3.3 V

2. ESP32-C3 Sensor nodes:
Each ESP32-C3 operates as an independent sensor node connected to a single
magnetic door or window sensor.
  * Reads sensor data
  * Sends status updates to the ESP32-S3

Connections:
GPIO                          Function
----                          --------
GPIO 2                        Magnetic Sensor
GND                           Ground

3. Magnetic Sensors
  Detects wether the door/window is open or closed

  Signals
  State                      Signal
  -----                      ------
  Closed                     LOW
  Open                       HIGH

  Example:
  Door closed:
  [ Magnet ] [ Sensor ]

  Door open:
  [ Magnet ]         [ Sensor ]

4. ESP32-CAM
  Takes pictures or streams video:
    * Motion detection
    * Object recognition (experimental)  

  Communication
  * TCP sockets are used for communication between sensor nodes and the Home Hub. 
  * HTTP is used for communication with the ESP32-CAM module.

5. BME 280-sensor
    Measures:
    * Temperature
    * Pressure
    * Humidity

    Communication protocol:
    * I2C
    I2C Address:
    0x77

    Connection:
    BME 280            ESP32-S3
    -------            --------
    VCC                3v3
    GND                GND
    SDA                SDA
    SCL                SCL

      
Communication architecture:
---------------------------

WiFi Network:
ESP32-C3 ----\
              \
ESP32-C3 ------> ESP32-S3
              /
ESP32-C3 ----/

Communication protocol:
  * HTTP
  * TCP

Power Supply:
Input Voltage:
  5V DC
Operating Voltage:
  3.3 V (regulated on-board)

Pinout chart:
  Device               GPIO              Function
  ------              ------             --------
  ESP32-S3            GPIO 8             I2C SDA
  ESP32-S3            GPIO 9             I2C SCL
    
  ESP32-C3 #1         GPIO 2             Door Sensor #1
  ESP32-C3 #2         GPIO 2             Window Sensor
  ESP32-C3 #3         GPIO 2             Door Sensor #2

System limitations:
* The ESP32-CAM module is currently experimental and not fully integrated.
* Sensor nodes require a stable Wi-Fi connection.
* The system is intended for indoor use only.
* Environmental measurements are updated periodically.
