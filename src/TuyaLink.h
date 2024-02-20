#ifndef TUYA_LINK_H
#define TUYA_LINK_H

#include "tuya_cacert.h"
#include <functional>

#include "TuyaDebug.h"

#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define TUYA_MQTT_ENDPOINT_CHINA "m1.tuyacn.com"
#define TUYA_MQTT_ENDPOINT_CENTRAL_EUROPE "m1.tuyaeu.com"
#define TUYA_MQTT_ENDPOINT_WESTERN_AMERICA "m1.tuyaus.com"
#define TUYA_MQTT_ENDPOINT_INDIA "m1.tuyain.com"

class TuyaLink;

class PropertyValue {
	friend class TuyaLink;

public: 
	// TODO: restrict
	template <typename T> inline T as() {
		return value.as<T>();
	}

	template <typename T> inline bool is() {
		return value.is<T>();
	}

private:
	PropertyValue(JsonVariant value):value(value) {};
	JsonVariant value;

};

typedef std::function<void(TuyaLink&)> TuyaLinkConnectedCallback;
typedef std::function<void(TuyaLink&)> TuyaLinkDisconnectedCallback;
typedef std::function<bool(TuyaLink&, const char* name, PropertyValue value)> TuyaLinkOnPropertySetCallback;

class TuyaLink {
public:
	TuyaLink();
	bool begin(String productKey, String deviceId, String deviceSecret, const char* mqttEndpoint);
	void onConnected(TuyaLinkConnectedCallback onConnectedCallback) {
		this->onConnectedCallback = onConnectedCallback;
	};
	void onDisconnected(TuyaLinkDisconnectedCallback onDisconnectedCallback) {
		this->onDisconnectedCallback = onDisconnectedCallback;
	};
	void onPropertySet(TuyaLinkOnPropertySetCallback onPropertySetCallback) {
		this->onPropertySetCallback = onPropertySetCallback;
	}

	bool reportProperty(const String& name, auto value) {
		StaticJsonDocument<200> msg;
		initReportPropertyMessage(msg, name);
		msg["data"][name]["value"] = value;
		return report("thing/property/report", msg);
	}

	void loop();

private:
	WiFiClientSecure wifiClient;
	PubSubClient pubSub;
	String deviceId;
	String deviceSecret;
	TuyaLinkConnectedCallback onConnectedCallback = [](TuyaLink&){};
	TuyaLinkDisconnectedCallback onDisconnectedCallback = [](TuyaLink&){};
	TuyaLinkOnPropertySetCallback onPropertySetCallback = [](TuyaLink&, const char* msg, PropertyValue value){ return true; };

	bool reconnect();

	void initReportPropertyMessage(JsonDocument& doc, const String& property);

	bool report(const char* topic, const JsonDocument& doc);

	void processIncomingMessage(char* topic, uint8_t* message, unsigned int len);
};

#endif