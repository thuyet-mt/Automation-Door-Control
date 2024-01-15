#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <WiFiMulti.h>
#include <WiFi.h>

/******Khai báo cho Wifi và MQTT******/

#define WIFI_SSID "308A C9 2.4G"
#define WIFI_PASSWORD "12345678"

#define WIFI_SSID1 "P.1914"
#define WIFI_PASSWORD1 "12345678"

#define WIFI_SSID2 "VIETTEL_MAI PHUONG"
#define WIFI_PASSWORD2 "23452345"

// BROKER
#define MQTT_HOST IPAddress(192, 168, 1, 5)
#define MQTT_PORT 1883

// MQTT Publish Topics
#define MQTT_PUB_RFID "esp/door/register/RFID"
#define MQTT_PUB_KEYPAD "esp/door/register/keypad"
#define MQTT_PUB_FINGERPRINT "esp/door/register/fingerprint"

// MQTT check Topics
#define MQTT_PUB_CHECK_RFID "esp/door/check/RFID"
#define MQTT_PUB_CHECK_KEYPAD "esp/door/check/keypad"
#define MQTT_PUB_CHECK_FINGERPRINT "esp/door/check/fingerprint"

void connectToWiFiNetwork();
void connectToMqtt();
void WiFiEvent(WiFiEvent_t event);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttPublish(uint16_t packetId);
void setupConnection(void);