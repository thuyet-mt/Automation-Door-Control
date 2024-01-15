#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <Wire.h>

typedef struct
{
    int id;
    int confidence;
    int area;
    float scale;
    char type[10];
} FingerprintData;

void setUpFingerprint(void);
void registerFingerprint(void);
void taskCheckFingerprint(void *pvParameter);
void taskAddFingerprint(void *pvParameter);
void printHex(int num, int precision);