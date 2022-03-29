#define TUYA_DEBUG

#include <Arduino.h>
#include <ArduinoJson.h>
#include <TuyaLink.h>

#ifdef ESP8266
    #include <ESP8266WiFi.h>
#else
    #include <WiFi.h>
#endif

#define SWITCH_DP_ID_KEY "101"

#define TUYA_PRODUCT_KEY "yourPID"
#define TUYA_DEVICE_UUID "yourUUID"
#define TUYA_DEVICE_AUTHKEY "yourAuthKey"
#define SOFTWARE_VER "1.0.0"

#define WLAN_SSID "yourSSID"
#define WLAN_PASS "yourPass"

TuyaLink tuyaLink;

bool state = false;
bool needsReport = false;

void applyState() {
    DEBUG_TUYA("Switch state %d", state);
    digitalWrite(LED_BUILTIN, state ? LOW : HIGH); // LED is inverted
    digitalWrite(D1, state ? HIGH : LOW);
}

void ICACHE_RAM_ATTR toggle() {
    needsReport = true;
    state = !state;
    applyState();
}

/* DP data reception processing function */
void onDpReceive(TuyaLink& instance, const char* jsonDps) {
    DEBUG_TUYA("Data point download value:%s", jsonDps);

    StaticJsonDocument<256> doc;
    deserializeJson(doc, jsonDps);

    state = doc[SWITCH_DP_ID_KEY];
    applyState();

    /* Report the received data to synchronize the switch status. */
    instance.reportDp(jsonDps);
}

void reportIfNeeded() {
    if (needsReport) {
        char buffer[16];
        sprintf(buffer, "{" SWITCH_DP_ID_KEY ": %s}", state ? "true" : "false");
        needsReport = tuyaLink.reportDp(buffer);
    }
}

void setup() {
    Serial.begin(74880);

    WiFi.begin(WLAN_SSID, WLAN_PASS);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(D1, OUTPUT);

    pinMode(D2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(D2), toggle, FALLING);

    tuyaLink.onDpReceive(onDpReceive);
    tuyaLink.begin(TUYA_PRODUCT_KEY, TUYA_DEVICE_UUID, TUYA_DEVICE_AUTHKEY, SOFTWARE_VER);
}

void loop() {    
    tuyaLink.handle();
    reportIfNeeded();
}