#ifndef DMSManager_h
#define DMSManager_h

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <Update.h>

struct DMSOptions
{
    const char *dmsUrl;
    const char *apiKey;
    const char *ssid;
    const char *password;
};

class DMSManager
{
public:
    DMSManager();
    void setup(const DMSOptions &options);
    void loop();

private:
    DMSOptions _options;
    WiFiClientSecure _secureWifiClient;
    PubSubClient _mqttClient;
    Preferences _preferences;

    void configureWiFi();
    void configureMqtt();
    void configureRemoteConfig();
    void reconnectToMqtt();
    void callback(char *topic, byte *message, unsigned int length);
    void publishAliveEvent();
    void handleOTA(String softwareVersion);
    void activateInDMS();
    String getDeviceId();
    void putDefaultValuesInNVS();
    bool writeToNVS(const char *key, const String &value);
    String readFromNVS(const char *key);
    void turnOnLed(int pinNumber);
    void turnOffLed(int pinNumber);
    void hexStringToByteArray(String hexString, byte *byteArray, int byteArrayLength);
    void sayHello();
    String _mqttServerIp;
    unsigned int _mqttServerPort;
    String _mqttUser;
    String _mqttPassword;
    unsigned long _aliveInterval;
    String _deviceModel;
    String _deviceBoardType;
    int _internalLed;

    unsigned long _previousMillis;
    const char *_root_ca PROGMEM;
};

#endif
