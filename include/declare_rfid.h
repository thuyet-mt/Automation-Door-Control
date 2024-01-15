#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>


#define SS_PIN 5   // ESP32 pin GPIO5
#define RST_PIN 27 // ESP32 pin GPIO27

void registerRFID();
void taskCheckRFID(void *pvParameter);
void taskAddRFID(void *pvParameter);