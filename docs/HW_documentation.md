# System Overview

## Purpose

The purpose of the system is to provide a modular home monitoring solution consisting of distributed magnetic sensors, environmental monitoring and camera surveillance.

The system includes:

- Magnetic sensors for doors and windows connected to ESP32-C3 sensor nodes
- Temperature, pressure and humidity measurements using a BME280 sensor
- Wireless communication between the Home Hub and sensor nodes
- Camera-based monitoring using an ESP32-CAM module

## Architecture

```text
+----------------+      +----------------+
|    ESP32-S3    | I2C  |    BME280      |
|  Central Unit  |------| Temp/RH/Press  |
+-------+--------+      +----------------+
        |
        | Wi-Fi
        |
+-------+--------+      +----------------+
|   ESP32-C3 #1  |----->| Magnetic Sensor|
+----------------+      +----------------+

+----------------+      +----------------+
|   ESP32-C3 #2  |----->| Magnetic Sensor|
+----------------+      +----------------+

+----------------+      +----------------+
|   ESP32-C3 #3  |----->| Magnetic Sensor|
+----------------+      +----------------+

Experimental (Not yet fully integrated):

+----------------+      +----------------+
|   ESP32-CAM    |----->| Motion         |
| Camera Module  |      | Detection      |
+----------------+      +----------------+
```

# Hardware Components

## 1. ESP32-S3

The Home Hub is responsible for:

- GUI (LVGL)
- Communication with sensor nodes
- Managing sensor placement and configuration
- Acquiring environmental data from the BME280 sensor

### Technical Data

| Parameter | Value |
|------------|--------|
| Processor | Xtensa LX7 Dual-Core |
| Flash | 8 MB |
| RAM | 512 KB |
| Communication | Wi-Fi, Bluetooth |
| Voltage | 3.3 V |

### Display

| Parameter | Value |
|------------|--------|
| Resolution | 1024 x 600 |
| Interface | RGB565 |
| Touch Controller | GT911 |
| Touch Interface | I2C |
| Touch Address | 0x5D |

## 2. ESP32-C3 Sensor Nodes

Each ESP32-C3 operates as an independent sensor node connected to a single magnetic door or window sensor.

Responsibilities:

- Monitoring magnetic sensor state
- Sending status updates to the ESP32-S3

### Connections

| GPIO | Function |
|--------|----------|
| GPIO2 | Magnetic Sensor |
| GND | Ground |

## 3. Magnetic Sensors

Detect whether a door or window is open or closed.

### Signals

| State | Signal |
|--------|---------|
| Closed | LOW |
| Open | HIGH |

### Example

```text
Door Closed:
[ Magnet ] [ Sensor ]

Door Open:
[ Magnet ]         [ Sensor ]
```

## 4. ESP32-CAM

Provides:

- Motion detection
- Object recognition (experimental)

### Communication

- HTTP
- Wi-Fi

## 5. BME280 Sensor

Measures:

- Temperature
- Pressure
- Humidity

### Communication

- I2C

### Address

Configured I2C address: `0x77`

### Connections

| BME280 | ESP32-S3 |
|---------|-----------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO8 |
| SCL | GPIO9 |

# Communication Architecture

## Wi-Fi Network

```text
ESP32-C3 ----\
              \
ESP32-C3 ------> ESP32-S3
              /
ESP32-C3 ----/
```

### Protocols

- TCP sockets are used between sensor nodes and the Home Hub.
- HTTP is used for communication with the ESP32-CAM module.

## Signal Flow

### Door/Window Sensor Event
```text
  Magnetic Sensor
        |
        v
    ESP32-C3
        |    Wi-Fi/TCP
        v
    ESP32-S3
        |
        v
    GUI Update
```

### Environmental Sensor Event (BME280)
```text
      BME280
        |    I2C
        v
    ESP32-S3
        | 
        v
    GUI Update
```

### Camera Monitoring

> **Note:** The ESP32-CAM Subsystem is currently being worked into the program and this is just an example of how it could work

```text
    ESP32-CAM
        |    HTTP
        v
    ESP32-S3
        | 
        v
       GUI
```

# Power Supply

| Parameter | Value |
|------------|--------|
| Input Voltage | 5 V DC |
| Operating Voltage | 3.3 V |

# Pin Configuration

## Application GPIO Usage

| Device | GPIO | Function |
|---------|---------|----------|
| ESP32-S3 | GPIO4 | GT911 Interrupt |
| ESP32-S3 | GPIO8 | I2C SDA |
| ESP32-S3 | GPIO9 | I2C SCL |
| ESP32-C3 #1 | GPIO2 | Door Sensor #1 |
| ESP32-C3 #2 | GPIO2 | Window Sensor |
| ESP32-C3 #3 | GPIO2 | Door Sensor #2 |

## Display Interface GPIOs

The following GPIOs are reserved by the Waveshare LCD subsystem:

| GPIO | Function |
|---------|----------|
| GPIO3 | VSYNC |
| GPIO5 | Display Enable |
| GPIO7 | Pixel Clock |

## Additional Reserved Display GPIOs

The RGB display subsystem reserves additional GPIOs for data transfer:

- GPIO0
- GPIO1
- GPIO2
- GPIO10
- GPIO14
- GPIO17
- GPIO18
- GPIO21
- GPIO38
- GPIO39
- GPIO40 through GPIO48

For further information regarding the WaveShares ESP32-S3 display module, see the official documentation: 
https://docs.waveshare.com/ESP32-S3-Touch-LCD-7B#interface-description

# System Limitations

- ESP32-CAM integration is still experimental.
- Sensor nodes require a stable Wi-Fi connection.
- The system is intended for indoor use only.
- Environmental measurements are updated periodically.
