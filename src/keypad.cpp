#include "declare_keypad.h"
#include "declare_lcd.h"
#include "declare_mqtt.h"

extern LiquidCrystal_I2C lcd;
extern AsyncMqttClient mqttClient;
/***************************Khai báo cho key pad**************************************/
const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns
// define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

byte rowPins[ROWS] = {26, 25, 33, 32}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {13, 12, 14, 15}; // connect to the column pinouts of the keypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

char hexaKeys1[ROWS][1] = {
    {'A'},
    {'B'},
    {'C'},
    {'D'}};

byte rowPins1[ROWS] = {26, 25, 33, 32}; // connect to the row pinouts of the keypad
byte colPins1[1] = {15};                // connect to the column pinouts of the keypad
Keypad customKeypad1 = Keypad(makeKeymap(hexaKeys1), rowPins1, colPins1, ROWS, 1);

/*****************Khai báo password cho keypad*******************************************/
String password;       // chuỗi này để lưu mật khẩu người dùng tạo
String input_password; // chuỗi này để lưu tạm thời mật khẩu người dùng tạo

void registerPassword(void)
{
    lcd.clear();
    String input_passwordset;
    Serial.println("Create Password");
    lcd.setCursor(0, 0);
    lcd.print("Create Password");
    char key = '1';
    while (key != '#')
    {
        key = customKeypad.waitForKey();
        if (key != '#')
        {
            Serial.print(key);
            lcd.setCursor(input_passwordset.length(), 1);
            lcd.print(key);
            input_passwordset += key;
        }
    }
    password = input_passwordset;
    Serial.println();
    Serial.println("Create Done!!!");
    lcd.setCursor(0, 0);
    lcd.clear();
    lcd.print("Create Done!!!");
    Serial.print("Password: ");
    Serial.println(password);
    lcd.setCursor(0, 1);
    lcd.print(password);
    lcd.clear();
    lcd.setCursor(1, 1);
    lcd.print("Register done!!!");
    // Publish an MQTT message on topic esp32/BME2800/temperature
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_KEYPAD, 1, true, password.c_str());
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_KEYPAD, packetIdPub1);
    Serial.printf("Password publish:");
    Serial.printf("Password: %s\n", password.c_str());
}
uint8_t readNumberKeypad(void)
{
    uint8_t num = 0;
    while (num == 0)
    {
        char key = customKeypad.waitForKey();
        num = (uint8_t)key - 48;
        lcd.setCursor(0, 1);
        lcd.print(num);
        key = NO_KEY;
    }
    return num;
}
void taskCheckKeyPad(void *pvParameter)
{
    while (1)
    {
        char key = customKeypad.waitForKeyTimeout(10);
        if (key)
        {
            if (key != '#')
            {
                Serial.print(key);
                lcd.setCursor(input_password.length(), 0);
            }

            if (key == '*')
            {
                input_password = ""; // clear input password
            }
            else if (key == '#')
            {
                if (password == input_password)
                {
                    Serial.println();
                    Serial.println("The password is correct, ACCESS GRANTED!");
                    // DO WORK
                    lcd.clear();
                    lcd.setCursor(2, 1);
                    lcd.print("ACCESS GRANTED!");
                    // 1. Open Door
                    // 2. Publish mes to broker
                    String notification = "Correct Password: ";
                    notification = notification + input_password;
                    uint16_t packetIdPubRFID = mqttClient.publish(MQTT_PUB_CHECK_KEYPAD, 1, true, notification.c_str());
                }
                else
                {
                    Serial.println();
                    Serial.println("The password is incorrect, ACCESS DENIED!");
                    lcd.clear();
                    lcd.setCursor(2, 1);
                    lcd.print("ACCESS DENIED!");
                    String notification = "Incorrect Password: ";
                    notification = notification + input_password;
                    uint16_t packetIdPubRFID = mqttClient.publish(MQTT_PUB_CHECK_KEYPAD, 1, true, notification.c_str());
                }

                input_password = ""; // clear input password
            }
            else
            {
                input_password += key; // append new character to input password string
            }
        }
        else
        {
            Serial.println("No key pressed");
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
    }
}
