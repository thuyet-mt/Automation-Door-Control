#include "declare_rfid.h"
#include "declare_mqtt.h"
#include "declare_keypad.h"
#include "declare_lcd.h"

extern Keypad customKeypad1;
extern LiquidCrystal_I2C lcd;
extern AsyncMqttClient mqttClient;
extern TaskHandle_t taskCheckRFIDHandle;
extern TaskHandle_t taskCheckKeyPadHandle;
extern TaskHandle_t taskCheckFingerprintIDHandle;
extern TaskHandle_t taskAddRFIDHandle;
extern TaskHandle_t taskAddFingerprintHandle;

MFRC522 rfid(SS_PIN, RST_PIN);
byte keyTagUID[10][4];
uint8_t numTags;

void registerRFID()
{
    Serial.println("Number RFID you add?");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Number RFID:");
    numTags = readNumberKeypad();
    Serial.println(numTags);
    lcd.setCursor(14, 0);
    lcd.print(numTags);
    delay(300);

    for (int i = 0; i < numTags; i++)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scan RFID to Reg");
        Serial.println("Scan RFID to Reg");
        lcd.setCursor(17, 0);
        lcd.print(i + 1);
        Serial.println(i + 1);
        delay(300);
        Serial.println("Check PICC_IsNewCardPresent");
        while (keyTagUID[i][0] == 0 || keyTagUID[i][1] == 0 || keyTagUID[i][2] == 0 || keyTagUID[i][3] == 0)
        { // Block everything while waiting for a RFID TAG to register
            if (rfid.PICC_IsNewCardPresent())
            { // new tag is available
                Serial.println("PICC_IsNewCardPresent");
                if (rfid.PICC_ReadCardSerial())
                { // NUID has been readed
                    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
                    keyTagUID[i][0] = rfid.uid.uidByte[0];
                    Serial.print(keyTagUID[i][0] < 0x10 ? " 0" : " ");
                    Serial.print(keyTagUID[i][0], HEX);
                    keyTagUID[i][1] = rfid.uid.uidByte[1];
                    Serial.print(keyTagUID[i][1] < 0x10 ? " 0" : " ");
                    Serial.print(keyTagUID[i][1], HEX);
                    keyTagUID[i][2] = rfid.uid.uidByte[2];
                    Serial.print(keyTagUID[i][2] < 0x10 ? " 0" : " ");
                    Serial.print(keyTagUID[i][2], HEX);
                    keyTagUID[i][3] = rfid.uid.uidByte[3];
                    Serial.print(keyTagUID[i][3] < 0x10 ? " 0" : " ");
                    Serial.print(keyTagUID[i][3], HEX);
                    // Display UID TAG on LCD 20x4
                    lcd.setCursor(0, 2);
                    lcd.print("UID:");

                    lcd.setCursor(5, 2);
                    lcd.print(keyTagUID[i][0] < 0x10 ? " 0" : " ");
                    lcd.print(keyTagUID[i][0], HEX);

                    lcd.setCursor(8, 2);
                    lcd.print(keyTagUID[i][1] < 0x10 ? " 0" : " ");
                    lcd.print(keyTagUID[i][1], HEX);

                    lcd.setCursor(11, 2);
                    lcd.print(keyTagUID[i][2] < 0x10 ? " 0" : " ");
                    lcd.print(keyTagUID[i][2], HEX);

                    lcd.setCursor(14, 2);
                    lcd.print(keyTagUID[i][3] < 0x10 ? " 0" : " ");
                    lcd.print(keyTagUID[i][3], HEX);
                    lcd.setCursor(0, 3);
                    lcd.print("Save UID TAG");
                    lcd.setCursor(13, 3);
                    lcd.print(i + 1);
                    Serial.println();
                    delay(300);
                }
                else
                {
                    Serial.println("PICC_ReadCardSerial Failed");
                }
                rfid.PICC_HaltA();      // halt PICC
                rfid.PCD_StopCrypto1(); // stop encryption on PCD
            }
            else
            {
                Serial.println("Wait for NewCardPresent");
                delay(500);
            }
        }
        String uidString; // để publish lên mqtt
        uidString = "";
        uidString += keyTagUID[i][0] < 0x10 ? " 0" : " ";
        uidString += String(keyTagUID[i][0], HEX);

        uidString += keyTagUID[i][1] < 0x10 ? " 0" : " ";
        uidString += String(keyTagUID[i][1], HEX);

        uidString += keyTagUID[i][2] < 0x10 ? " 0" : " ";
        uidString += String(keyTagUID[i][2], HEX);

        uidString += keyTagUID[i][3] < 0x10 ? " 0" : " ";
        uidString += String(keyTagUID[i][3], HEX);
        // mqttClient.publish(MQTT_PUB_RFID, uidString.c_str());

        char uidString_char[30];
        sprintf(uidString_char, "%x%x%x%x", keyTagUID[i][0], keyTagUID[i][1], keyTagUID[i][2], keyTagUID[i][3]);
        uint16_t packetIdPubRFID = mqttClient.publish(MQTT_PUB_RFID, 1, true, uidString_char);
        Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_RFID, packetIdPubRFID);
        Serial.printf("RFID TAG Register: ");
        Serial.printf(uidString_char);
        if (i < (numTags - 1))
        {
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print("Next UID Register");
            delay(300);
        }
        else
        {
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print("Register UID Done !");
        }
    }
    delay(300);
}
void taskCheckRFID(void *pvParameter)
{
    while (1)
    {
        // Đọc UID thẻ
        // Serial.println("Check PICC_IsNewCardPresent");
        if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
        {
            Serial.println("PICC_IsNewCardPresent");
            // Duyệt mảng keyTagUID để tìm UID trùng khớp
            bool accessGranted = false;
            Serial.println("Check keyTagUID");
            for (int i = 0; i < 10; i++)
            {
                if (rfid.uid.uidByte[0] == keyTagUID[i][0] && rfid.uid.uidByte[1] == keyTagUID[i][1] &&
                    rfid.uid.uidByte[2] == keyTagUID[i][2] && rfid.uid.uidByte[3] == keyTagUID[i][3])
                {
                    accessGranted = true;
                    break;
                }
            }
            Serial.println("Check keyTagUID done");
            if (accessGranted)
            {
                // Cho phép truy cập
                Serial.println("Access granted");
                lcd.clear();
                lcd.setCursor(2, 1);
                lcd.print("ACCESS GRANTED");
                vTaskDelay(300 / portTICK_PERIOD_MS);
                lcd.clear();
                char uidString_char[30];
                sprintf(uidString_char, "%x%x%x%x", rfid.uid.uidByte[0], rfid.uid.uidByte[1], rfid.uid.uidByte[2], rfid.uid.uidByte[3]);
                char notification[] = "Correct UID TAG: ";
                strcat(notification, uidString_char);
                uint16_t packetIdPubRFID = mqttClient.publish(MQTT_PUB_CHECK_RFID, 1, true, notification);
            }
            else
            {
                // Từ chối truy cập
                Serial.print("Access denied");
                for (int i = 0; i < rfid.uid.size; i++)
                {
                    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
                    Serial.print(rfid.uid.uidByte[i], HEX);
                }
                lcd.clear();
                lcd.setCursor(2, 1);
                lcd.print("ACCESS DENIED");
                vTaskDelay(300 / portTICK_PERIOD_MS);
                lcd.clear();
                char uidString_char[30];
                sprintf(uidString_char, "%x%x%x%x", rfid.uid.uidByte[0], rfid.uid.uidByte[1], rfid.uid.uidByte[2], rfid.uid.uidByte[3]);
                char notification[] = "Incorrect UID TAG: ";
                strcat(notification, uidString_char);
                // uint16_t packetIdPubRFID = mqttClient.publish(MQTT_PUB_CHECK_RFID, 1, true, notification);
            }

            rfid.PICC_HaltA();
            rfid.PCD_StopCrypto1();
        }
        else
        {
            Serial.println("Wait for NewCardPresent");
            //  lcd.clear();
            //  lcd.setCursor(2, 1);
            //  lcd.print("PICC_IsNewCardPresent fail");
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}
void taskAddRFID(void *pvParameter)
{
    while (1)
    {
        // if (digitalRead(4))
        char key = customKeypad1.waitForKeyTimeout(100);
        if (key == 'A')
        {
            vTaskSuspend(taskCheckFingerprintIDHandle);
            vTaskSuspend(taskCheckKeyPadHandle);
            vTaskSuspend(taskCheckRFIDHandle);
            vTaskSuspend(taskAddFingerprintHandle);
            Serial.println("Number RFID you add?");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Number RFID:");
            uint8_t addTags;
            addTags = 1;
            Serial.println(addTags);
            lcd.setCursor(14, 0);
            lcd.print(addTags);
            vTaskDelay(300 / portTICK_PERIOD_MS);

            for (int i = numTags; i < numTags + addTags; i++)
            {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Scan RFID to Reg");
                Serial.println("Scan RFID to Reg");
                lcd.setCursor(17, 0);
                lcd.print(i + 1);
                Serial.println(i + 1);
                vTaskDelay(100 / portTICK_PERIOD_MS);
                Serial.println("Check PICC_IsNewCardPresent");
                while (keyTagUID[i][0] == 0 || keyTagUID[i][1] == 0 || keyTagUID[i][2] == 0 || keyTagUID[i][3] == 0)
                { // Block everything while waiting for a RFID TAG to register
                    if (rfid.PICC_IsNewCardPresent())
                    { // new tag is available
                        Serial.println("PICC_IsNewCardPresent");
                        if (rfid.PICC_ReadCardSerial())
                        { // NUID has been readed
                            MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
                            keyTagUID[i][0] = rfid.uid.uidByte[0];
                            Serial.print(keyTagUID[i][0] < 0x10 ? " 0" : " ");
                            Serial.print(keyTagUID[i][0], HEX);
                            keyTagUID[i][1] = rfid.uid.uidByte[1];
                            Serial.print(keyTagUID[i][1] < 0x10 ? " 0" : " ");
                            Serial.print(keyTagUID[i][1], HEX);
                            keyTagUID[i][2] = rfid.uid.uidByte[2];
                            Serial.print(keyTagUID[i][2] < 0x10 ? " 0" : " ");
                            Serial.print(keyTagUID[i][2], HEX);
                            keyTagUID[i][3] = rfid.uid.uidByte[3];
                            Serial.print(keyTagUID[i][3] < 0x10 ? " 0" : " ");
                            Serial.print(keyTagUID[i][3], HEX);
                            // Display UID TAG on LCD 20x4
                            lcd.setCursor(0, 2);
                            lcd.print("UID:");

                            lcd.setCursor(5, 2);
                            lcd.print(keyTagUID[i][0] < 0x10 ? " 0" : " ");
                            lcd.print(keyTagUID[i][0], HEX);

                            lcd.setCursor(8, 2);
                            lcd.print(keyTagUID[i][1] < 0x10 ? " 0" : " ");
                            lcd.print(keyTagUID[i][1], HEX);

                            lcd.setCursor(11, 2);
                            lcd.print(keyTagUID[i][2] < 0x10 ? " 0" : " ");
                            lcd.print(keyTagUID[i][2], HEX);

                            lcd.setCursor(14, 2);
                            lcd.print(keyTagUID[i][3] < 0x10 ? " 0" : " ");
                            lcd.print(keyTagUID[i][3], HEX);
                            lcd.setCursor(0, 3);
                            lcd.print("Save UID TAG");
                            lcd.setCursor(13, 3);
                            lcd.print(i + 1);
                            Serial.println();
                            vTaskDelay(100/portTICK_PERIOD_MS);
                        }
                        else
                        {
                            Serial.println("PICC_ReadCardSerial Failed");
                        }
                        rfid.PICC_HaltA();      // halt PICC
                        rfid.PCD_StopCrypto1(); // stop encryption on PCD
                    }
                    else
                    {
                        Serial.println("Wait for NewCardPresent");
                        vTaskDelay(300 / portTICK_PERIOD_MS);
                    }
                }
                String uidString; // để publish lên mqtt
                uidString = "";
                uidString += keyTagUID[i][0] < 0x10 ? " 0" : " ";
                uidString += String(keyTagUID[i][0], HEX);

                uidString += keyTagUID[i][1] < 0x10 ? " 0" : " ";
                uidString += String(keyTagUID[i][1], HEX);

                uidString += keyTagUID[i][2] < 0x10 ? " 0" : " ";
                uidString += String(keyTagUID[i][2], HEX);

                uidString += keyTagUID[i][3] < 0x10 ? " 0" : " ";
                uidString += String(keyTagUID[i][3], HEX);
                // mqttClient.publish(MQTT_PUB_RFID, uidString.c_str());

                char uidString_char[30];
                sprintf(uidString_char, "%x%x%x%x", keyTagUID[i][0], keyTagUID[i][1], keyTagUID[i][2], keyTagUID[i][3]);
                uint16_t packetIdPubRFID = mqttClient.publish(MQTT_PUB_RFID, 1, true, uidString_char);
                Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_RFID, packetIdPubRFID);
                Serial.printf("RFID TAG Register: ");
                Serial.printf(uidString_char);
                if (i < (numTags - 1))
                {
                    lcd.clear();
                    lcd.setCursor(0, 1);
                    lcd.print("Next UID Register");
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }
                else
                {
                    lcd.clear();
                    lcd.setCursor(0, 1);
                    lcd.print("Register UID Done !");
                }
            }
            numTags += addTags;
            vTaskDelay(100 / portTICK_PERIOD_MS);
            vTaskResume(taskCheckFingerprintIDHandle);
            vTaskResume(taskCheckKeyPadHandle);
            vTaskResume(taskCheckRFIDHandle);
            vTaskResume(taskAddFingerprintHandle);
        }
        else
        {
            Serial.println("Wait for key A pressed");
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }
}
