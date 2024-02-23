#define TUYA_DEBUG

#include <Arduino.h>
#ifdef ESP8266
    #include <ESP8266WiFi.h>
#else
    #include <WiFi.h>
#endif

#define TUYA_ENABLE_AP_CONFIG_MODE 1

#include <TuyaLink.h>

#define BUTTON_PIN 0 // it is the "flash" button in both esp32 and esp8266 node mcus

#define TUYA_PRODUCT_ID "yourPID"
#define TUYA_DEVICE_ID "yourID"
#define TUYA_DEVICE_SECRET "yourSecret"

TuyaLink tuyaLink;

bool enterConfig = false;

void ICACHE_RAM_ATTR toggle() {
    enterConfig = true;
}

void setup() {
#ifdef ESP8266  
    Serial.begin(74880);
#else
    Serial.begin(115200);
#endif

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    WiFi.begin();
    tuyaLink.begin(TUYA_PRODUCT_ID, TUYA_DEVICE_ID, TUYA_DEVICE_SECRET, TUYA_MQTT_ENDPOINT_WESTERN_AMERICA);

    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), toggle, FALLING);

}

void loop() {
    if (enterConfig) {
        tuyaLink.beginApConfigMode();
        enterConfig = false;
    }
    tuyaLink.loop();
}

