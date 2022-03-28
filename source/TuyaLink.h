#ifndef TUYA_LINK_H
#define TUYA_LINK_H

#include "tuya_iot.h"
#include <functional>

#include "TuyaDebug.h"

#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#ifdef ESP8266
    #include <ESP8266HTTPClient.h>
#else
    #include <HTTPClient.h>
#endif

class TuyaLink;

typedef std::function<void(TuyaLink&)> TuyaLinkConnectedCallback;
typedef std::function<void(TuyaLink&)> TuyaLinkDisconnectedCallback;
typedef std::function<void(TuyaLink&, const char*)> TuyaLinkDPReceiveCallback;

class TuyaLink {
public:
	TuyaLink();
	bool begin(const char* productKey, const char* uuid, const char* authKey, const char* softwareVer);
	void onConnected(TuyaLinkConnectedCallback onConnectedCallback) {
		this->onConnectedCallback = onConnectedCallback;
	};
	void onDisconnected(TuyaLinkDisconnectedCallback onDisconnectedCallback) {
		this->onDisconnectedCallback = onDisconnectedCallback;
	};
	void onDpReceive(TuyaLinkDPReceiveCallback onDpReceiveCallback) {
		this->onDpReceiveCallback = onDpReceiveCallback;
	}
	bool reportDp(const char* jsonString);
	void handle();

private:
	tuya_iot_client_t client;
	TuyaLinkConnectedCallback onConnectedCallback = [](TuyaLink&){};
	TuyaLinkDisconnectedCallback onDisconnectedCallback = [](TuyaLink&){};
	TuyaLinkDPReceiveCallback onDpReceiveCallback = [](TuyaLink&, const char*){};
	void handleEvent(tuya_event_msg_t* event);
};

#endif