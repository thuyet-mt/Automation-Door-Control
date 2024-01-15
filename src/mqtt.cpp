#include "declare_mqtt.h"

WiFiMulti wifiMulti;
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;
unsigned long previousMillis = 0; // Stores last time a message was published
const long interval = 10000;      // Interval at which to publish values

/************Function cho WiFi & MQTT***********/
void connectToWiFiNetwork()
{
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
    wifiMulti.addAP(WIFI_SSID1, WIFI_PASSWORD1);
    wifiMulti.addAP(WIFI_SSID2, WIFI_PASSWORD2);

    // Kết nối với mạng WiFi mạnh nhất
    if (wifiMulti.run() == WL_CONNECTED)
    {
        Serial.println("");
        Serial.print("WiFi connected to: ");
        Serial.println(WiFi.SSID());
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
}
void connectToMqtt()
{
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}
void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}
void onMqttConnect(bool sessionPresent)
{
    Serial.println("Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);
}
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.println("Disconnected from MQTT.");
    if (WiFi.isConnected())
    {
        xTimerStart(mqttReconnectTimer, 0);
    }
}
void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
    Serial.println("Subscribe acknowledged.");
    Serial.print(" packetId: ");
    Serial.println(packetId);
    Serial.print(" qos: ");
    Serial.println(qos);
}
void onMqttUnsubscribe(uint16_t packetId)
{
    Serial.println("Unsubscribe acknowledged.");
    Serial.print(" packetId: ");
    Serial.println(packetId);
}
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    // Do whatever you want when you receive a message
    // Save the message in a variable
    String receivedMessage;
    for (int i = 0; i < len; i++)
    {
        Serial.println((char)payload[i]);
        receivedMessage += (char)payload[i];
    }
    // Save topic in a String variable
    String receivedTopic = String(topic);
    Serial.print("Received Topic: ");
    Serial.println(receivedTopic);
    // Check which GPIO we want to control
    int stringLen = receivedTopic.length();
    // Get the index of the last slash
    int lastSlash = receivedTopic.lastIndexOf("/");
    // Get the GPIO number (it's after the last slash "/")
    // esp/digital/GPIO
    String gpio = receivedTopic.substring(lastSlash + 1, stringLen);
    Serial.print("DIGITAL GPIO: ");
    Serial.println(gpio);
    Serial.print("STATE: ");
    Serial.println(receivedMessage);
    // Check if it is DIGITAL
    if (receivedTopic.indexOf("digital") > 0)
    {
        // Set the specified GPIO as output
        pinMode(gpio.toInt(), OUTPUT);
        // Control the GPIO
        if (receivedMessage == "true")
        {
            digitalWrite(gpio.toInt(), HIGH);
        }
        else
        {
            digitalWrite(gpio.toInt(), LOW);
        }
    }
    Serial.println("Publish received.");
    Serial.print(" topic: ");
    Serial.println(topic);
    Serial.print(" qos: ");
    Serial.println(properties.qos);
    Serial.print(" dup: ");
    Serial.println(properties.dup);
    Serial.print(" retain: ");
    Serial.println(properties.retain);
    Serial.print(" len: ");
    Serial.println(len);
    Serial.print(" index: ");
    Serial.println(index);
    Serial.print(" total: ");
    Serial.println(total);
}
void onMqttPublish(uint16_t packetId)
{
    Serial.println("Publish acknowledged.");
    Serial.print(" packetId: ");
    Serial.println(packetId);
}
void setupConnection(void)
{
    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(4000), pdFALSE,
                                      (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
    wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(4000), pdFALSE,
                                      (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWiFiNetwork));
    WiFi.onEvent(WiFiEvent);
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    // mqttClient.setCredentials(BROKER_USER, BROKER_PASS);
    connectToWiFiNetwork();
}
