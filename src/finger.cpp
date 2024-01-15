#include "declare_keypad.h"
#include "declare_finger.h"
#include "declare_lcd.h"

#define mySerial Serial2

extern Keypad customKeypad1;
extern LiquidCrystal_I2C lcd;

extern TaskHandle_t taskCheckRFIDHandle;
extern TaskHandle_t taskCheckKeyPadHandle;
extern TaskHandle_t taskCheckFingerprintIDHandle;
extern TaskHandle_t taskAddRFIDHandle;
extern TaskHandle_t taskAddFingerprintHandle;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id;    // id này nhận số từ keypad phục vụ cho finger print
uint8_t sumID; // tổng số ID cho finger print
/******************************biến phục vụ cho hai hàm fingerprint*/
uint8_t global_p;
int global_id;
// Định nghĩa cấu trúc JSON lưu dữ liệu vân tay để publish


FingerprintData fingerInfo;

void setUpFingerprint(void)
{
    while (!Serial)
        ; // For Yun/Leo/Micro/Zero/...
    delay(300);
    Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

    // set the data rate for the sensor serial port
    finger.begin(57600);

    if (finger.verifyPassword())
    {
        Serial.println("Found fingerprint sensor!");
    }
    else
    {
        Serial.println("Did not find fingerprint sensor :(");
        while (1)
        {
            delay(1);
        }
    }
    finger.emptyDatabase();
    Serial.println(F("Empty Database of sensor"));
    Serial.println(F("Reading sensor parameters"));
    finger.getParameters();
    Serial.print(F("Status: 0x"));
    Serial.println(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x"));
    Serial.println(finger.system_id, HEX);
    Serial.print(F("Capacity: "));
    Serial.println(finger.capacity);
    Serial.print(F("Security level: "));
    Serial.println(finger.security_level);
    Serial.print(F("Device address: "));
    Serial.println(finger.device_addr, HEX);
    Serial.print(F("Packet len: "));
    Serial.println(finger.packet_len);
    Serial.print(F("Baud rate: "));
    Serial.println(finger.baud_rate);
}
void registerFingerprint(void)
{
    Serial.println("Ready to enroll a fingerprint!");
    Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ready to enroll");
    lcd.setCursor(2, 1);
    lcd.print("fingerprint !!");
    delay(300);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Number SUM ID: ");
    lcd.setCursor(0, 1);
    lcd.print("for finger register");
    // (from 1 to 127)
    sumID = readNumberKeypad();
    while (sumID == 0)
    {
        Serial.println("ID #0 not allowed, try again!");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("ID #0 not allowed");
        lcd.setCursor(5, 2);
        lcd.print("Try again!");
        delay(500);
        lcd.clear();
    }
    lcd.setCursor(16, 0);
    lcd.print(sumID);
    Serial.print("Sum ID #");
    Serial.println(sumID);
    delay(300);
    for (id = 1; id <= sumID; id++)
    {
        int p = -1;
        Serial.print("Waiting for valid finger to enroll as #");
        Serial.println(id);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Wait valid finger");
        lcd.setCursor(0, 1);
        lcd.print("to enroll as #");
        lcd.setCursor(15, 1);
        lcd.print(id);
        while (p != FINGERPRINT_OK)
        {
            p = finger.getImage();
            Serial.print("p=");
            Serial.print(p < 0x10 ? " 0" : " ");
            Serial.println(p, HEX);
            switch (p)
            {
            case FINGERPRINT_OK:
                Serial.println("Image taken");
                lcd.clear();
                lcd.setCursor(3, 1);
                lcd.print("Image taken");
                delay(300);
                break;
            case FINGERPRINT_NOFINGER:
                Serial.println(".");
                break;
            case FINGERPRINT_PACKETRECIEVEERR:
                Serial.println("Communication error");
                break;
            case FINGERPRINT_IMAGEFAIL:
                Serial.println("Imaging error");
                break;
            default:
                Serial.println("Unknown error");
                break;
            }
        }

        // OK success!
        p = finger.image2Tz(1);
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Image converted");
            break;
        case FINGERPRINT_IMAGEMESS:
            Serial.println("Image too messy");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            break;
        case FINGERPRINT_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }

        Serial.println("Remove finger");
        lcd.clear();
        lcd.setCursor(3, 1);
        lcd.print("Remove finger");
        delay(300);
        p = 0;
        while (p != FINGERPRINT_NOFINGER)
        {
            p = finger.getImage();
        }
        Serial.print("ID ");
        Serial.println(id);
        p = -1;
        Serial.println("Place same finger again");
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Place finger again");
        while (p != FINGERPRINT_OK)
        {
            p = finger.getImage();
            switch (p)
            {
            case FINGERPRINT_OK:
                Serial.println("Image taken");
                lcd.clear();
                lcd.setCursor(3, 1);
                lcd.print("Image taken");
                delay(300);
                break;
            case FINGERPRINT_NOFINGER:
                Serial.print(".");
                break;
            case FINGERPRINT_PACKETRECIEVEERR:
                Serial.println("Communication error");
                break;
            case FINGERPRINT_IMAGEFAIL:
                Serial.println("Imaging error");
                break;
            default:
                Serial.println("Unknown error");
                break;
            }
        }

        // OK success!

        p = finger.image2Tz(2);
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Image converted");
            break;
        case FINGERPRINT_IMAGEMESS:
            Serial.println("Image too messy");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            break;
        case FINGERPRINT_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }

        // OK converted!
        Serial.print("Creating model for #");
        Serial.println(id);

        p = finger.createModel();
        if (p == FINGERPRINT_OK)
        {
            Serial.println("Prints matched!");
        }
        else if (p == FINGERPRINT_PACKETRECIEVEERR)
        {
            Serial.println("Communication error");
            break;
        }
        else if (p == FINGERPRINT_ENROLLMISMATCH)
        {
            Serial.println("Fingerprints did not match");
            break;
        }
        else
        {
            Serial.println("Unknown error");
            break;
        }

        Serial.print("ID ");
        Serial.println(id);
        p = finger.storeModel(id);
        if (p == FINGERPRINT_OK)
        {
            Serial.println("Stored!");
            uint8_t p1 = finger.loadModel(id);
            Serial.print("Template ");
            Serial.print(id);
            Serial.println(" loaded");
            p1 = finger.getModel();
            switch (p1)
            {
            case FINGERPRINT_OK:
                Serial.print("Template ");
                Serial.print(id);
                Serial.println(" transferring:");
                break;
            default:
                Serial.print("Unknown error ");
                Serial.println(p1);
                break;
            }
            // one data packet is 267 bytes. in one data packet, 11 bytes are 'usesless' :D
            uint8_t bytesReceived[534]; // 2 data packets
            memset(bytesReceived, 0xff, 534);

            uint32_t starttime = millis();
            int i = 0;
            while (i < 534 && (millis() - starttime) < 20000)
            {
                if (mySerial.available())
                {
                    bytesReceived[i++] = mySerial.read();
                }
            }
            Serial.print(i);
            Serial.println(" bytes read.");
            Serial.println("Decoding packet...");

            uint8_t fingerTemplate[512]; // the real template
            memset(fingerTemplate, 0xff, 512);

            // filtering only the data packets
            int uindx = 9, index = 0;
            memcpy(fingerTemplate + index, bytesReceived + uindx, 256); // first 256 bytes
            uindx += 256;                                               // skip data
            uindx += 2;                                                 // skip checksum
            uindx += 9;                                                 // skip next header
            index += 256;                                               // advance pointer
            memcpy(fingerTemplate + index, bytesReceived + uindx, 256); // second 256 bytes

            for (int i = 0; i < 512; ++i)
            {
                // Serial.print("0x");
                printHex(fingerTemplate[i], 2);
                // Serial.print(", ");
            }
            Serial.println("\ndone.");
            // fingerInfo.id = id;
            // fingerInfo.area = finger.image2Tz(2);
            // fingerInfo.confidence = finger.confidence;
            // doc["id"] = id;
            // for (int i = 0; i < 512; ++i)
            // {
            //     doc["template"][i] = fingerTemplate[i];
            // }
            // String jsonString;
            // serializeJson(doc, jsonString);
            // uint16_t packetIdPubRFID = mqttClient.publish(MQTT_PUB_FINGERPRINT, 1, true, jsonString.c_str());
        }
        else if (p == FINGERPRINT_PACKETRECIEVEERR)
        {
            Serial.println("Communication error");
            break;
        }
        else if (p == FINGERPRINT_BADLOCATION)
        {
            Serial.println("Could not store in that location");
            break;
        }
        else if (p == FINGERPRINT_FLASHERR)
        {
            Serial.println("Error writing to flash");
            break;
        }
        else
        {
            Serial.println("Unknown error");
            break;
        }
    }
    Serial.println("Register done!!!");
    lcd.clear();
    lcd.setCursor(1, 1);
    lcd.print("Register done!!!");
}
void printHex(int num, int precision)
{
    char tmp[16];
    char format[128];

    sprintf(format, "%%.%dX", precision);

    sprintf(tmp, format, num);
    Serial.print(tmp);
}
void taskCheckFingerprint(void *pvParameter)
{
    while (1)
    {
        // Loop until a finger is detected
        if ((finger.getImage() == FINGERPRINT_OK))
        {
            Serial.println("Image taken");
            if ((finger.image2Tz() == FINGERPRINT_OK))
            {
                Serial.println("Image converted");
                if ((finger.fingerSearch() == FINGERPRINT_OK))
                {
                    Serial.print("Found ID #");
                    Serial.print(finger.fingerID);
                    Serial.print(" with confidence of ");
                    Serial.println(finger.confidence);
                    if (0 < finger.fingerID && finger.fingerID <= sumID && finger.confidence > 50)
                    {
                        lcd.clear();
                        lcd.setCursor(2, 1);
                        lcd.print("ACCESS GRANTED!");
                        delay(300);
                        lcd.clear();
                    }
                    else
                    {
                        lcd.clear();
                        lcd.setCursor(2, 1);
                        lcd.print("ACCESS DENIDED!");
                        delay(300);
                        lcd.clear();
                    }
                }
                else
                {
                    Serial.println("Cannot search finger");
                    lcd.clear();
                    lcd.setCursor(2, 1);
                    lcd.print("ACCESS DENIDED!");
                    delay(300);
                    lcd.clear();
                }
            }
            else
            {
                Serial.println("Cannot convert image");
            }
        }
        else
        {
            Serial.println("Finger is not detected. Cannot take image");
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }
}
void taskAddFingerprint(void *pvParameter)
{

    while (1)
    {
        // if (digitalRead(15))
        char key = customKeypad1.waitForKeyTimeout(100);
        if (key == 'B')
        {
            vTaskSuspend(taskCheckFingerprintIDHandle);
            vTaskSuspend(taskCheckKeyPadHandle);
            vTaskSuspend(taskCheckRFIDHandle);
            vTaskSuspend(taskAddRFIDHandle);
            Serial.println("Ready to enroll a fingerprint!");
            Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Ready to enroll");
            lcd.setCursor(2, 1);
            lcd.print("fingerprint !!");
            delay(300);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("SUM ID: ");
            lcd.setCursor(0, 1);
            lcd.print("for finger adding");
            lcd.setCursor(11, 0);
            lcd.print(sumID);
            // (from 1 to 127)
            uint8_t addID = 1;
            while (addID == 0)
            {
                Serial.println("ID #0 not allowed, try again!");
                lcd.clear();
                lcd.setCursor(0, 1);
                lcd.print("ID #0 not allowed");
                lcd.setCursor(5, 2);
                lcd.print("Try again!");
                delay(500);
                lcd.clear();
            }
            lcd.setCursor(16, 0);
            lcd.print(addID);
            Serial.print("Add ID #");
            Serial.println(addID);
            delay(300);
            for (id = 1 + sumID; id <= sumID + addID; id++)
            {
                int p = -1;
                Serial.print("Waiting for valid finger to enroll as #");
                Serial.println(id);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Wait valid finger");
                lcd.setCursor(0, 1);
                lcd.print("to enroll as #");
                lcd.setCursor(15, 1);
                lcd.print(id);
                while (p != FINGERPRINT_OK)
                {
                    p = finger.getImage();
                    Serial.print("p=");
                    Serial.print(p < 0x10 ? " 0" : " ");
                    Serial.println(p, HEX);
                    switch (p)
                    {
                    case FINGERPRINT_OK:
                        Serial.println("Image taken");
                        lcd.clear();
                        lcd.setCursor(3, 1);
                        lcd.print("Image taken");
                        delay(300);
                        break;
                    case FINGERPRINT_NOFINGER:
                        Serial.println(".");
                        break;
                    case FINGERPRINT_PACKETRECIEVEERR:
                        Serial.println("Communication error");
                        break;
                    case FINGERPRINT_IMAGEFAIL:
                        Serial.println("Imaging error");
                        break;
                    default:
                        Serial.println("Unknown error");
                        break;
                    }
                }

                // OK success!
                p = finger.image2Tz(1);
                switch (p)
                {
                case FINGERPRINT_OK:
                    Serial.println("Image converted");
                    break;
                case FINGERPRINT_IMAGEMESS:
                    Serial.println("Image too messy");
                    break;
                case FINGERPRINT_PACKETRECIEVEERR:
                    Serial.println("Communication error");
                    break;
                case FINGERPRINT_FEATUREFAIL:
                    Serial.println("Could not find fingerprint features");
                    break;
                case FINGERPRINT_INVALIDIMAGE:
                    Serial.println("Could not find fingerprint features");
                    break;
                default:
                    Serial.println("Unknown error");
                    break;
                }

                Serial.println("Remove finger");
                lcd.clear();
                lcd.setCursor(3, 1);
                lcd.print("Remove finger");
                delay(300);
                p = 0;
                while (p != FINGERPRINT_NOFINGER)
                {
                    p = finger.getImage();
                }
                Serial.print("ID ");
                Serial.println(id);
                p = -1;
                Serial.println("Place same finger again");
                lcd.clear();
                lcd.setCursor(0, 1);
                lcd.print("Place finger again");
                while (p != FINGERPRINT_OK)
                {
                    p = finger.getImage();
                    switch (p)
                    {
                    case FINGERPRINT_OK:
                        Serial.println("Image taken");
                        lcd.clear();
                        lcd.setCursor(3, 1);
                        lcd.print("Image taken");
                        delay(300);
                        break;
                    case FINGERPRINT_NOFINGER:
                        Serial.print(".");
                        break;
                    case FINGERPRINT_PACKETRECIEVEERR:
                        Serial.println("Communication error");
                        break;
                    case FINGERPRINT_IMAGEFAIL:
                        Serial.println("Imaging error");
                        break;
                    default:
                        Serial.println("Unknown error");
                        break;
                    }
                }

                // OK success!

                p = finger.image2Tz(2);
                switch (p)
                {
                case FINGERPRINT_OK:
                    Serial.println("Image converted");
                    break;
                case FINGERPRINT_IMAGEMESS:
                    Serial.println("Image too messy");
                    break;
                case FINGERPRINT_PACKETRECIEVEERR:
                    Serial.println("Communication error");
                    break;
                case FINGERPRINT_FEATUREFAIL:
                    Serial.println("Could not find fingerprint features");
                    break;
                case FINGERPRINT_INVALIDIMAGE:
                    Serial.println("Could not find fingerprint features");
                    break;
                default:
                    Serial.println("Unknown error");
                    break;
                }

                // OK converted!
                Serial.print("Creating model for #");
                Serial.println(id);

                p = finger.createModel();
                if (p == FINGERPRINT_OK)
                {
                    Serial.println("Prints matched!");
                }
                else if (p == FINGERPRINT_PACKETRECIEVEERR)
                {
                    Serial.println("Communication error");
                    break;
                }
                else if (p == FINGERPRINT_ENROLLMISMATCH)
                {
                    Serial.println("Fingerprints did not match");
                    break;
                }
                else
                {
                    Serial.println("Unknown error");
                    break;
                }

                Serial.print("ID ");
                Serial.println(id);
                p = finger.storeModel(id);
                if (p == FINGERPRINT_OK)
                {
                    Serial.println("Stored!");
                    uint8_t p1 = finger.loadModel(id);
                    Serial.print("Template ");
                    Serial.print(id);
                    Serial.println(" loaded");
                    p1 = finger.getModel();
                    switch (p1)
                    {
                    case FINGERPRINT_OK:
                        Serial.print("Template ");
                        Serial.print(id);
                        Serial.println(" transferring:");
                        break;
                    default:
                        Serial.print("Unknown error ");
                        Serial.println(p1);
                        break;
                    }
                    // one data packet is 267 bytes. in one data packet, 11 bytes are 'usesless' :D
                    uint8_t bytesReceived[534]; // 2 data packets
                    memset(bytesReceived, 0xff, 534);

                    uint32_t starttime = millis();
                    int i = 0;
                    while (i < 534 && (millis() - starttime) < 20000)
                    {
                        if (mySerial.available())
                        {
                            bytesReceived[i++] = mySerial.read();
                        }
                    }
                    Serial.print(i);
                    Serial.println(" bytes read.");
                    Serial.println("Decoding packet...");

                    uint8_t fingerTemplate[512]; // the real template
                    memset(fingerTemplate, 0xff, 512);

                    // filtering only the data packets
                    int uindx = 9, index = 0;
                    memcpy(fingerTemplate + index, bytesReceived + uindx, 256); // first 256 bytes
                    uindx += 256;                                               // skip data
                    uindx += 2;                                                 // skip checksum
                    uindx += 9;                                                 // skip next header
                    index += 256;                                               // advance pointer
                    memcpy(fingerTemplate + index, bytesReceived + uindx, 256); // second 256 bytes

                    for (int i = 0; i < 512; ++i)
                    {
                        // Serial.print("0x");
                        printHex(fingerTemplate[i], 2);
                        // Serial.print(", ");
                    }
                    Serial.println("\ndone.");
                    fingerInfo.id = id;
                    fingerInfo.area = finger.image2Tz(2);
                    fingerInfo.confidence = finger.confidence;
                    // doc["id"] = id;
                    // for (int i = 0; i < 512; ++i)
                    // {
                    //     doc["template"][i] = fingerTemplate[i];
                    // }
                    // String jsonString;
                    // serializeJson(doc, jsonString);
                    // uint16_t packetIdPubRFID = mqttClient.publish(MQTT_PUB_FINGERPRINT, 1, true, jsonString.c_str());
                }
                else if (p == FINGERPRINT_PACKETRECIEVEERR)
                {
                    Serial.println("Communication error");
                    break;
                }
                else if (p == FINGERPRINT_BADLOCATION)
                {
                    Serial.println("Could not store in that location");
                    break;
                }
                else if (p == FINGERPRINT_FLASHERR)
                {
                    Serial.println("Error writing to flash");
                    break;
                }
                else
                {
                    Serial.println("Unknown error");
                    break;
                }
            }
            sumID += addID;
            Serial.println("Register done!!!");
            lcd.clear();
            lcd.setCursor(1, 1);
            lcd.print("Register done!!!");
            delay(100);
            vTaskResume(taskCheckFingerprintIDHandle);
            vTaskResume(taskCheckKeyPadHandle);
            vTaskResume(taskCheckRFIDHandle);
            vTaskResume(taskAddRFIDHandle);
        }
        else
        {
            Serial.println("Wait for key B pressed");
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }
}
