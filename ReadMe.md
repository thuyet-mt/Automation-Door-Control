# ESP32 Smart Door Control

## Introduction

This project implements a smart door lock system using the ESP32 board. The system supports three authentication methods: password entry via a keypad, fingerprint recognition, and RFID card.

## Hardware

The hardware components used in this project include:

- ESP32 DevKitC V4 controller
- 20x4 LCD display connected via I2C for status display and notifications
- 4x4 matrix keypad for password entry
- AS608 optical fingerprint sensor for fingerprint registration and recognition
- MFRC522 RFID reader for RFID card registration and recognition

## Software Features

The device provides an LCD menu for setting up a 4-6 character password via the keypad, registering fingerprints, and registering RFID cards. The data is temporarily stored in memory, but can be extended to be stored in SPIFFS or EEPROM.

## Authentication

Three FreeRTOS tasks run in parallel to monitor the keypad, fingerprint sensor, and RFID reader. Successful authentication displays "Access Granted" on the LCD and sends a notification via MQTT. If authentication fails, it displays "Access Denied".

## MQTT Integration

The device connects to MQTT over WiFi to send notifications each time authentication occurs and to register new passwords, fingerprints, and RFID cards.

## Future Improvements

Future improvements could include storing registration data in non-volatile memory, adding BLE connectivity in place of WiFi+MQTT, designing a 3D printed case, and controlling a servo to simulate door unlocking.

## Installation

To install this project, follow these steps:

```bash
# Clone the repository
git clone https://github.com/thuyet-mt14072508/ESP32-Smart-Door-Control.git

# Navigate to the directory
cd ESP32-Smart-Door-Control

# Install dependencies
# Add the necessary steps here

Usage
To use this project, follow these steps:

# Add the necessary steps here

Contributing
Contributions are welcome! Please read the contributing guidelines before getting started.

License
This project is licensed under the MIT License.
