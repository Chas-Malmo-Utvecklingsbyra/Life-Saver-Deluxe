# Life-Saver-Deluxe

Home security and environmental monitoring system built on ESP32 microcontrollers.

Developed by **Chas Malmö Utvecklingsbyrå**.

---

## Demo

### Home-Hub

Home Tab provides real-time sensor monitoring:

<img width="723" height="413" alt="20260612_105901-ezgif com-crop" src="https://github.com/user-attachments/assets/0e128615-3bd0-4fa0-aac5-8db8b47f5a9e" />

Environmental monitoring:

<img width="3438" height="1848" alt="20260612_114712" src="https://github.com/user-attachments/assets/f8620dab-ef8a-48e0-992a-fc6c1ecf3ffc" />

Settings configuration:

<img width="3048" height="1538" alt="20260612_110601" src="https://github.com/user-attachments/assets/fda9c2bf-53c3-459c-87dd-e54211a5f328" />

### Cam-module:
<img width="1134" height="951" alt="image (1)" src="https://github.com/user-attachments/assets/d3be9410-5c6a-4df9-b1a6-689a1f15f170" />

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
        ^
        |
        | Wi-Fi
        |
+-------+--------+
|   ESP32-C3 #1  |
| Door Sensor #1 |
+----------------+

+----------------+
|   ESP32-C3 #2  |
| Window Sensor  |
+----------------+

+----------------+
|   ESP32-C3 #3  |
| Door Sensor #2 |
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
* TCP/IP (Magnetic Sensor)
* HTTP (Cam)

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

- [Hardware Documentation](docs/HW_documentation.md)
  Which includes Sequence Diagrams, Signal Flow Diagrams and Pin Configuration and more.

---

## Sequence diagram

<img width="605" height="1055" alt="Skärmbild 2026-06-12 104923" src="https://github.com/user-attachments/assets/3c7319bf-10a1-4c8a-8c27-3dc10d04e74f" />

---

## Development Process

The project follows an iterative SCRUM-inspired development process.

Project duration:

* Start: Week 14
* Final Delivery: Week 25

---

## Key Learnings

This project involved:

- Embedded systems development using ESP-IDF
- FreeRTOS task management
- LVGL GUI development
- TCP/IP communication between ESP32 devices
- I2C sensor integration
- Hardware debugging and validation

---

## Team Members

- [Emilio Ganibegovic](https://github.com/AlCapone1234)
- [Pär Lundh](https://github.com/lundhpargmailcom)
- [Henrik Westerlund](https://github.com/Henrik-Westerlund)
- [Lukas Städe](https://github.com/HoffaQt)
- [Isa Shipshani](https://github.com/isashiphotmailcom)
