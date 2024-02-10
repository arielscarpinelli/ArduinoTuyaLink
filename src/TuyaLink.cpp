#include "TuyaLink.h"

#include "signature.h"

TuyaLink::TuyaLink():pubSub(wifiClient) {}

bool TuyaLink::begin(String productKey, 
	String deviceId, 
	String deviceSecret) {

    this->deviceId = deviceId;
    this->deviceSecret = deviceSecret;

#ifdef ESP8266
    X509List cert((const uint8_t*) tuya_cacert_pem, strlen(tuya_cacert_pem));
    
    configTime(0, 0, "pool.ntp.org");
    wifiClient.setTrustAnchors(&cert);
#else    
	wifiClient.setCACert(tuya_cacert_pem);
#endif    
    pubSub.setServer("m1.tuyacn.com", 8883);
	pubSub.setCallback([this](char* topic, uint8_t* message, unsigned int len) {
        this->onMessageCallback(*this, topic);
	});

    return reconnect();
}

bool TuyaLink::reconnect() {
    char clientId[128];
    char username[128];
    char password[128];

	if(tuya_mqtt_auth_signature_calculate(deviceId.c_str(), deviceSecret.c_str(), clientId, username, password) != 0) {
        DEBUG_TUYA("failed to calculate signature");
        return false;
    };


    if(!pubSub.connect(clientId, username, password)) {
        DEBUG_TUYA("failed to connect");
        return false;
    }

    DEBUG_TUYA("connected");
    
    char topic[128];
    sprintf(topic, "tylink/%s/channel/downlink/auto_subscribe", deviceId.c_str());
	if(!pubSub.subscribe(topic, 1)) {
        DEBUG_TUYA("failed to subscribe");
        return false;
    };

    DEBUG_TUYA("subscribed");

	return true;
}



bool TuyaLink::reportProperty(const char* jsonString) {

    char topic[128];
    sprintf(topic, "tylink/%s/thing/property/report", deviceId);

    char payload[256];
    sprintf(payload, "{\"data\": %s}", jsonString);

    return pubSub.publish(topic, payload);
}

void TuyaLink::loop() {
    if (!pubSub.loop()) {
        reconnect();
    }
}