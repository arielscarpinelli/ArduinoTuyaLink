#ifndef TUYA_LINK_H
#define TUYA_LINK_H

#include "tuya_cacert.h"
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
typedef std::function<void(TuyaLink&, const char*)> TuyaLinkOnMessageCallback;

class TuyaLink {
public:
	TuyaLink();
	bool begin(String productKey, String deviceId, String deviceSecret);
	void onConnected(TuyaLinkConnectedCallback onConnectedCallback) {
		this->onConnectedCallback = onConnectedCallback;
	};
	void onDisconnected(TuyaLinkDisconnectedCallback onDisconnectedCallback) {
		this->onDisconnectedCallback = onDisconnectedCallback;
	};
	void onMessage(TuyaLinkOnMessageCallback onMessageCallback) {
		this->onMessageCallback = onMessageCallback;
	}
	bool reportProperty(const char* jsonString);
	void loop();

private:
	WiFiClientSecure wifiClient;
	PubSubClient pubSub;
	String deviceId;
	String deviceSecret;
	TuyaLinkConnectedCallback onConnectedCallback = [](TuyaLink&){};
	TuyaLinkDisconnectedCallback onDisconnectedCallback = [](TuyaLink&){};
	TuyaLinkOnMessageCallback onMessageCallback = [](TuyaLink&, const char* msg){};

	bool reconnect();
};

#endif