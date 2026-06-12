# Life-Saver-Deluxe

Home security and environmental monitoring system built on ESP32 microcontrollers.

Developed by **Chas Malmö Utvecklingsbyrå**.

---

## Overview

Life-Saver Deluxe is a distributed home monitoring system consisting of a central Home Hub and multiple wireless sensor nodes.

The system monitors:

* Door and window status using magnetic sensors
* Temperature
* Humidity
* Air pressure
* Camera-based monitoring (experimental)

The architecture is designed around ESP32 devices communicating over Wi-Fi.

---

## Features

### Home Hub (ESP32-S3)

* LVGL-based graphical user interface
* Sensor management
* Sensor placement management
* Environmental monitoring
* Wi-Fi communication
* Event logging

### Sensor Nodes (ESP32-C3)

* Magnetic door/window sensors
* Low-latency status updates
* Wireless communication with Home Hub

### Environmental Monitoring

* Temperature measurement
* Humidity measurement
* Air pressure measurement

### Camera Module (Experimental)

* Motion detection
* Object recognition
* Image capture

---

## System Architecture

```text
+----------------+      +----------------+
|    ESP32-S3    | I2C  |    BME280      |
|  Home Hub      |------| Temp/Hum/Press |
+-------+--------+      +----------------+
        |
        | Wi-Fi
        |
+-------+--------+
|   ESP32-C3 #1  |
| Door Sensor    |
+----------------+

+----------------+
|   ESP32-C3 #2  |
| Window Sensor  |
+----------------+

+----------------+
|   ESP32-C3 #3  |
| Door Sensor    |
+----------------+

+----------------+
|   ESP32-CAM    |
| Experimental   |
+----------------+
```

---

## Technology Stack

* ESP-IDF
* FreeRTOS
* LVGL
* C
* C++
* Wi-Fi Networking
* TCP/IP
* HTTP

---

## Repository Structure

```text
Life-Saver-Deluxe/
│
├── Embedded_Core/
│   └── Shared embedded framework
│
├── Home_Hub/
│   └── ESP32-S3 application
│
├── Magnetic_Sensor/
│   └── ESP32-C3 sensor firmware
│
├── docs/
│   └── Project documentation
│
└── tests/
    └── Unit tests
```

---

## Hardware Requirements

### Home Hub

* ESP32-S3
* Display
* BME280 Sensor

### Sensor Node

* ESP32-C3
* Magnetic Reed Sensor

### Optional

* ESP32-CAM

---

## Building the Project

### Prerequisites

* ESP-IDF
* Python 3.x
* Git

### Clone Repository

```bash
git clone https://github.com/Chas-Malmo-Utvecklingsbyra/Life-Saver-Deluxe.git
cd Life-Saver-Deluxe
```

### Build

```bash
idf.py build
```

### Flash

```bash
idf.py flash
```

### Monitor

```bash
idf.py monitor
```

---

## Documentation

Additional documentation can be found in the `/docs` directory.

Included documentation:

* Hardware Documentation
* Sequence Diagrams
* Signal Flow Diagrams

---

## Development Process

The project follows an iterative SCRUM-inspired development process.

Project duration:

* Start: Week 14
* Final Delivery: Week 25

---

## Team Members

* Emilio Ganibegovic
* Pär Lundh
* Henrik Westerlund
* Lukas Städe
* Isa Shipshani
