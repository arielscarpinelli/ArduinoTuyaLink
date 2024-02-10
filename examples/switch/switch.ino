#define TUYA_DEBUG

#include <Arduino.h>
#ifdef ESP8266
    #include <ESP8266WiFi.h>
#else
    #include <WiFi.h>
#endif

#ifdef ESP32
  #define LED_BUILTIN 2
#endif  

#define BUTTON_PIN 0 // it is the "flash" button in both esp32 and esp8266 node mcus

#include <TuyaLink.h>

#define SWITCH_DP_ID_KEY "switch_1"

#define TUYA_PRODUCT_ID "yourPID"
#define TUYA_DEVICE_ID "yourID"
#define TUYA_DEVICE_SECRET "yourSecret"

#define WLAN_SSID "yourSSID"
#define WLAN_PASS "yourPass"

TuyaLink tuyaLink;

bool state = false;
bool needsReport = false;

void applyState() {
    DEBUG_TUYA("Switch state %d", state);
#ifdef ESP8266    
    digitalWrite(LED_BUILTIN, state ? LOW : HIGH); // LED is inverted
#else
    digitalWrite(LED_BUILTIN, state);
#endif    
}

void ICACHE_RAM_ATTR toggle() {
    needsReport = true;
    state = !state;
    applyState();
}

void onMessage(TuyaLink& instance, const char* msg) {
    DEBUG_TUYA("on message %s", msg);
}

void reportIfNeeded() {
    if (needsReport) {
        char buffer[32];
        sprintf(buffer, "{" SWITCH_DP_ID_KEY ": %s}", state ? "true" : "false");
        DEBUG_TUYA("Reporting %s", buffer);
        needsReport = !tuyaLink.reportProperty(buffer);
    }
}

void setup() {
#ifdef ESP8266  
    Serial.begin(74880);
#else
    Serial.begin(115200);
#endif

    DEBUG_TUYA("Switch example. Welcome!");
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WLAN_SSID, WLAN_PASS);

    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), toggle, FALLING);

    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
    }

    tuyaLink.onMessage(onMessage);
    tuyaLink.begin(TUYA_PRODUCT_ID, TUYA_DEVICE_ID, TUYA_DEVICE_SECRET);


}

void loop() {
    tuyaLink.loop();
    reportIfNeeded();
}
