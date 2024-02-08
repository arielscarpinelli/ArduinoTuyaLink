#include "TuyaLink.h"

#include "signature.h"

TuyaLink::TuyaLink():pubSub(wifiClient) {}

bool TuyaLink::begin(const char* productKey, 
	const char* deviceId, 
	const char* deviceSecret) {

    this->deviceId = deviceId;

	wifiClient.setCACert(tuya_cacert_pem);
    pubSub.setServer("m1.tuyacn.com", 8883);
	pubSub.setCallback([this](char* topic, uint8_t* message, unsigned int len) {
        this->onMessageCallback(*this, topic);
	});

    char clientId[128];
    char username[128];
    char password[128];

	if(tuya_mqtt_auth_signature_calculate(deviceId, deviceSecret, clientId, username, password) != 0) {
        return false;
    };


    if(!pubSub.connect(clientId, username, password)) {
        DEBUG_TUYA("failed to connect");
        return false;
    }
    
    char topic[128];
    sprintf(topic, "tylink/%s/channel/downlink/auto_subscribe", deviceId);
	if(!pubSub.subscribe(topic, 1)) {
        DEBUG_TUYA("failed to subscribe");
        return false;
    };

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
    pubSub.loop();
}