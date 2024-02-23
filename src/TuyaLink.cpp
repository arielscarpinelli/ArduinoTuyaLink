#include "TuyaLink.h"

#include "signature.h"

TuyaLink::TuyaLink():pubSub(wifiClient) {}

bool TuyaLink::begin(String productKey, 
	String deviceId, 
	String deviceSecret,
    const char* mqttEndpoint) {

    this->deviceId = deviceId;
    this->deviceSecret = deviceSecret;

#ifdef ESP8266
    X509List cert((const uint8_t*) tuya_cacert_pem, strlen(tuya_cacert_pem));
    
    configTime(0, 0, "pool.ntp.org");
    wifiClient.setTrustAnchors(&cert);
#else    
	wifiClient.setCACert(tuya_cacert_pem);
#endif    
    pubSub.setServer(mqttEndpoint, 8883);
	pubSub.setCallback([this](char* topic, uint8_t* message, unsigned int len) {
        this->processIncomingMessage(topic, message, len);
	});

    return reconnect();
}

bool TuyaLink::reconnect() {
    char clientId[128];
    char username[128];
    char password[128];

    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

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

bool TuyaLink::loop() {
    switch(configMode) {
        case TUYA_CONFIG_MODE_NONE:
        default:
            if (!pubSub.loop()) {
                return reconnect();
            }
            return true;
#ifdef TUYA_ENABLE_AP_CONFIG_MODE
        case TUYA_CONFIG_MODE_AP:
            return loopApConfig();
#endif            
        case TUYA_CONFIG_MODE_SMARTCONFIG:
            // TODO
            return true;
    }
}

#ifdef TUYA_ENABLE_AP_CONFIG_MODE
bool TuyaLink::beginApConfigMode() {
    DEBUG_TUYA("begin ap config mode");
    if(!WiFi.softAP("SL-12345")) {
        DEBUG_TUYA("cannot start AP mode");
        return false;
    }
    if(!udp.begin(TUYA_UDP_CONFIG_PORT)) {
        DEBUG_TUYA("cannot initialize udp");
        return false;
    }
    configMode = TUYA_CONFIG_MODE_AP;
    return true;
}

bool TuyaLink::loopApConfig() {
    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
        char incomingPacket[packetSize + 1];
        int len = udp.read(incomingPacket, packetSize);
        if (len > 0) {
            DEBUG_TUYA("Received %d bytes from %s, port %d", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
            char* configStart = (char*)memchr(incomingPacket, '{', len);
            char* configEnd = (char*)memrchr(incomingPacket, '}', len);

            if (!configStart || !configEnd) {
                DEBUG_TUYA("received packet has no json");
                return false;
            }

            DEBUG_TUYA("got json %s", configStart);

            configEnd[1] = NULL;

            StaticJsonDocument<256> doc;
            DeserializationError err = deserializeJson(doc, configStart);
            if (err) {
                DEBUG_TUYA("failed to deserialize json: %s ", err.c_str());
                return false;
            }

            if (!doc.containsKey("ssid") || !doc.containsKey("passwd")) {
                DEBUG_TUYA("json does not contain ssid or passwd");
                return false;
            }

            String ssid = doc["ssid"];
            String passwd = doc["passwd"];
            String token = doc["token"];

            WiFi.mode(WIFI_STA);
            WiFi.persistent(true);
            WiFi.begin(ssid, passwd);
            configMode = TUYA_CONFIG_MODE_NONE;
            return true;
        } else {
            DEBUG_TUYA("failed to read incoming packet of size %d", packetSize);
        }
    }
    return false;
}
#endif

void TuyaLink::initReportPropertyMessage(JsonDocument& doc, const String& property) {

    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t time = (uint32_t)tv.tv_sec;

    doc["time"] = time;
    
    String msgId = String(time);
    msgId.concat(tv.tv_usec);
    doc["msgId"] = msgId;

	JsonObject data = doc.createNestedObject("data");
    JsonObject prop = data.createNestedObject(property);
	prop["time"] = time;
}

bool TuyaLink::report(const char* topicTail, const JsonDocument& doc) {

    char topic[128];
    sprintf(topic, "tylink/%s/%s", deviceId.c_str(), topicTail);

    char buffer[256];
    size_t n = serializeJson(doc, buffer);

    DEBUG_TUYA("Reporting %.*s to %s", n, buffer, topic);

    return pubSub.publish(topic, buffer, n);
}

void TuyaLink::processIncomingMessage(char* topic, uint8_t* message, unsigned int len) {
    // We are not supporting sub-devices. As such, we ignore checking the device id in the topic.
    if (strstr(topic, "thing/property/set") != NULL) {
        StaticJsonDocument<256> doc;
        DeserializationError err = deserializeJson(doc, (char*)message, len);
        if (err) {
            DEBUG_TUYA("processIncomingMessage failed to deserialize json: %s for topic: %s message: %.*s", err.c_str(), topic, len, message);
            return;
        }

        auto data = doc["data"];
        if (data == nullptr || !data.is<JsonObject>()) {
            DEBUG_TUYA("data field not found or invalid for topic: %s message: %.*s",  topic, len, message);
        }

        bool ok = true;
        for(JsonPair prop : data.as<JsonObject>()) {
            ok = ok && this->onPropertySetCallback(*this, prop.key().c_str(), PropertyValue(prop.value()));
        }
    }
}
