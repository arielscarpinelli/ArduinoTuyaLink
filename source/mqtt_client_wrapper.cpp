
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "TuyaDebug.h"

extern "C" {

#include "mqtt_client_interface.h"

}

class MQTTContext {
public:
	WiFiClientSecure client;
	PubSubClient pubSub;
	BearSSL::X509List cert;
	mqtt_client_config_t config;
	MQTTContext():pubSub(client) {}
};

void* mqtt_client_new(void) {
	return new MQTTContext();
}

void  mqtt_client_free(void* client) {
	delete (MQTTContext*) client;
}

mqtt_client_status_t mqtt_client_init(void* client, const mqtt_client_config_t* config) {
	MQTTContext* context = (MQTTContext*) client;
	context->config = *config;
	context->pubSub.setServer(config->host, config->port);
	context->pubSub.setCallback([client](char* topic, uint8_t* message, unsigned int len) {
		MQTTContext* context = (MQTTContext*) client;
		if (context->config.on_message) {
			mqtt_client_message msg = {
				.payload = message,
				.length = len
			};
			context->config.on_message(client, 0, &msg, context->config.userdata);
		}
	});
	context->cert.append(config->cacert, config->cacert_len);
  	context->client.setTrustAnchors(&context->cert);
	return MQTT_STATUS_SUCCESS;
}

mqtt_client_status_t mqtt_client_deinit(void* client) {
	return MQTT_STATUS_SUCCESS;
}

mqtt_client_status_t mqtt_client_connect(void* client) {
	MQTTContext* context = (MQTTContext*) client;
	if (context->pubSub.connect(context->config.clientid, context->config.username, context->config.password)) {
		return MQTT_STATUS_SUCCESS;
	} else {
		DEBUG_TUYA("connection failed: %d", context->pubSub.state());
		return MQTT_STATUS_CONNECT_FAILED;
	}
}

mqtt_client_status_t mqtt_client_disconnect(void* client) {
	((MQTTContext*) client)->pubSub.disconnect();
}

mqtt_client_status_t mqtt_client_yield(void* client) {
	((MQTTContext*) client)->pubSub.loop();
}

uint16_t mqtt_client_subscribe(void* client, const char* topic, uint8_t qos) {
	((MQTTContext*) client)->pubSub.subscribe(topic, qos);
}

uint16_t mqtt_client_unsubscribe(void* client, const char* topic, uint8_t qos) {
	((MQTTContext*) client)->pubSub.unsubscribe(topic);
}

uint16_t mqtt_client_publish(void* client, const char* topic, const uint8_t* payload, size_t length, uint8_t qos) {
	((MQTTContext*) client)->pubSub.publish(topic, payload, length);
}