#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <WiFiUdp.h>
#include "mbedtls/aes.h"
#include "mbedtls/base64.h"
#include <cstring>
#include <HTTPClient.h>
#include <Preferences.h>
#include <Update.h>
#include <DMSManager.h>

DMSManager::DMSManager() : _mqttClient(_secureWifiClient) {}

void DMSManager::setup(const DMSOptions &options)
{
    sayHello();
    _options = options;
    _previousMillis = 0;
    _root_ca = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";
    int isReady = 0;
    while (isReady != 1)
    {

        configureWiFi();
        putDefaultValuesInNVS();
        activateInDMS();
        configureRemoteConfig();
        pinMode(_internalLed, OUTPUT);
        turnOnLed(_internalLed);
        configureMqtt();
        turnOffLed(_internalLed);
        isReady = 1;
    }
}

void DMSManager::loop()
{
    if (!_mqttClient.connected())
    {
        reconnectToMqtt();
    }

    _mqttClient.loop();
    unsigned long currentMillis = millis();
    if (currentMillis - _previousMillis >= _aliveInterval)
    {
        _previousMillis = currentMillis;
        publishAliveEvent();
    }
}

void DMSManager::configureWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(_options.ssid, _options.password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void DMSManager::configureMqtt()
{
    _secureWifiClient.setCACert(_root_ca);
    _mqttClient.setServer(_mqttServerIp.c_str(), _mqttServerPort);
    _mqttClient.setCallback([this](char *topic, byte *payload, unsigned int length)
                            { this->callback(topic, payload, length); });
    reconnectToMqtt();
}

void DMSManager::reconnectToMqtt()
{
    while (!_mqttClient.connected())
    {
        String clientId = "ESP32Client-" + getDeviceId();
        if (_mqttClient.connect(clientId.c_str(), _mqttUser.c_str(), _mqttPassword.c_str()))
        {
            Serial.println("MQTT connected.");
            String rebootTopic = "/api/" + getDeviceId() + "/reboot";
            _mqttClient.subscribe(rebootTopic.c_str());
            String updateTopic = "/api/" + getDeviceId() + "/update";
            _mqttClient.subscribe(updateTopic.c_str());
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(_mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void DMSManager::callback(char *topic, byte *message, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    char msg[length + 1];
    memcpy(msg, message, length);
    msg[length] = '\0';
    const size_t capacity = JSON_OBJECT_SIZE(10);
    DynamicJsonDocument parsed(capacity);
    DeserializationError error = deserializeJson(parsed, (const char *)msg);

    if (error)
    {
        Serial.println("Failed to parse JSON");
        Serial.println(error.c_str());
        return;
    }

    String deviceId = getDeviceId();
    String rebootTopic = "/api/" + deviceId + "/reboot";
    String updateTopic = "/api/" + deviceId + "/update";

    if (String(topic) == rebootTopic)
    {
        Serial.println("Reboot command received");
        ESP.restart();
    }
    else if (String(topic) == updateTopic)
    {
        Serial.println("Update command received");
        String softwareVersion = parsed["software_version"].as<String>();
        handleOTA(softwareVersion);
    }
}

void DMSManager::turnOnLed(int pinNumber)
{
    digitalWrite(pinNumber, HIGH);
}

void DMSManager::turnOffLed(int pinNumber)
{
    digitalWrite(pinNumber, LOW);
}

void DMSManager::activateInDMS()
{
    String deviceToken = getDeviceId();

    String url = String(_options.dmsUrl) + "/api/activate/" + deviceToken;

    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Api-Key", _options.apiKey);

    int httpResponseCode = http.POST("{}");

    if (httpResponseCode == 201)
    {
        Serial.println("Device activated in DMS");
    }
    else if (httpResponseCode == 400)
    {
        Serial.println("Device already activated in DMS");
    }
    else
    {
        Serial.print("Error while activating device CODE:");
        Serial.println(httpResponseCode);
    }

    http.end();
}

void DMSManager::putDefaultValuesInNVS()
{
    String deviceToken = readFromNVS("device_id");
    if (deviceToken == "")
    {
        Serial.println("Device token is empty in NVS, putting default");
        String deviceId = getDeviceId();
        writeToNVS("device_id", deviceId);
    }
}

String DMSManager::readFromNVS(const char *key)
{
    if (!_preferences.begin("NVS", true))
    {
        Serial.println("NVS initialization error");
        return "";
    }

    if (!_preferences.isKey(key))
    {
        Serial.println("Key not found in NVS");
        Serial.println(key);
        _preferences.end();
        return "";
    }

    String value = _preferences.getString(key, "");
    _preferences.end();
    return value;
}

bool DMSManager::writeToNVS(const char *key, const String &value)
{
    if (!_preferences.begin("NVS", false))
    {
        Serial.println("NVS initialization error");
        return false;
    }

    if (!_preferences.putString(key, value.c_str()))
    {
        Serial.println("NVS write error");
        _preferences.end();
        return false;
    }

    _preferences.end();
    return true;
}

String DMSManager::getDeviceId()
{
    uint8_t baseMac[6];
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    char baseMacChr[18] = {0};
    sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X",
            baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);

    String guid = String(baseMacChr);
    guid.replace(":", "");
    return guid;
}

void DMSManager::publishAliveEvent()
{
    Serial.println("Reporting alive");
    StaticJsonDocument<120> json;
    char event[120];
    json["deviceId"] = getDeviceId();
    serializeJson(json, event);
    _mqttClient.publish("/alive", event);
}

void DMSManager::configureRemoteConfig()
{
    String deviceToken = getDeviceId();
    Serial.println("Device token: " + deviceToken);
    String url = String(_options.dmsUrl) + "/api/configuration/" + deviceToken;

    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Api-Key", _options.apiKey);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
        String payload = http.getString();

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);

        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return;
        }

        _mqttServerIp = doc["MqttBrokerHost"].as<String>();
        _mqttServerPort = doc["MqttBrokerPort"].as<unsigned int>();
        _mqttUser = doc["MqttUsername"].as<String>();
        _mqttPassword = doc["MqttPassword"].as<String>();
        _aliveInterval = doc["AliveInterval"].as<unsigned long>();
        _deviceModel = doc["DeviceModel"].as<String>();
        _deviceBoardType = doc["DeviceBoardType"].as<String>();
        _internalLed = doc["InternalLedPin"].as<unsigned int>();
    }
    else
    {
        Serial.print("Error while getting config device CODE: ");
        Serial.println(httpResponseCode);
    }

    http.end();
}

void DMSManager::handleOTA(String softwareVersion)
{
    Serial.println("Update started");

    StaticJsonDocument<200> doc;
    doc["software_version"] = softwareVersion;
    String body;
    serializeJson(doc, body);

    String url = String(_options.dmsUrl) + "/api/device/" + getDeviceId() + "/firmware?software_version=" + softwareVersion;
    HTTPClient http;
    http.begin(url);
    http.addHeader("X-Api-Key", _options.apiKey);
    
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        WiFiClient *client = http.getStreamPtr();
        size_t contentLength = http.getSize();

        if (Update.begin(contentLength))
        {
            size_t written = Update.writeStream(*client);
            if (written == contentLength)
            {
                Serial.println("Written : " + String(written) + " successfully");
            }
            else
            {
                Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
            }

            if (Update.end())
            {

                Serial.println("OTA done!");
                if (Update.isFinished())
                {
                    Serial.println("Update successfully completed. Rebooting.");
                    ESP.restart();
                }
                else
                {
                    Serial.println("Update not finished? Something went wrong!");
                }
            }
            else
            {
                Serial.println("Error Occurred. Error #: " + String(Update.getError()));
            }
        }
        else
        {
            Serial.println("Not enough space to begin OTA");
        }
    }
    else
    {
        Serial.println("Error on HTTP request");
    }
}

void DMSManager::sayHello()
{
    Serial.println(" /$$$$$$$  /$$      /$$  /$$$$$$");
    Serial.println("| $$__  $$| $$$    /$$$ /$$__  $$");
    Serial.println("| $$  \ $$| $$$$  /$$$$| $$  \__/");
    Serial.println("| $$  | $$| $$ $$/$$ $$|  $$$$$$");
    Serial.println("| $$  | $$| $$  $$$| $$ \____  $$");
    Serial.println("| $$  | $$| $$\  $ | $$ /$$  \ $$");
    Serial.println("| $$$$$$$/| $$ \/  | $$|  $$$$$$/");
    Serial.println("|_______/ |__/     |__/ \______/ ");
}