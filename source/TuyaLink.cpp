#include "TuyaLink.h"
#include <map>

std::map<tuya_iot_client_t*, TuyaLink*> instances;

TuyaLink::TuyaLink() {
    instances[&(this->client)] = this;
}

bool TuyaLink::begin(const char* productKey, 
	const char* uuid, 
	const char* authKey, 
	const char* softwareVer) {

    DEBUG_TUYA("TuyaLink::begin productKey:%s uuid:%s softwareVer:%s", uuid, authKey, softwareVer);

    tuya_iot_config_t config = {
        .productkey = productKey,
        .uuid = uuid,
        .authkey = authKey,
        .software_ver = softwareVer,
        .event_handler = [](tuya_iot_client_t* client, tuya_event_msg_t* event) {
            instances[client]->handleEvent(event);
        }
    };

    if (tuya_iot_init(&(this->client), &config) != OPRT_OK) {
        return false;
    }

	if (tuya_iot_start(&(this->client)) != OPRT_OK) {
		return false;
	}

	return true;
}

bool TuyaLink::reportDp(const char* jsonString) {
    return tuya_iot_dp_report_json(&(this->client), jsonString) == OPRT_OK;
}

void TuyaLink::handle() {
	tuya_iot_yield(&(this->client));
}

void TuyaLink::handleEvent(tuya_event_msg_t* event) {
    DEBUG_TUYA("Tuya Event ID:%d(%s)", event->id, EVENT_ID2STR(event->id));
    switch (event->id) {

    case TUYA_EVENT_MQTT_CONNECTED:
		this->onConnectedCallback(*this);
        break;

    case TUYA_EVENT_MQTT_DISCONNECT:
		this->onDisconnectedCallback(*this);
        break;

    case TUYA_EVENT_DP_RECEIVE:
    	this->onDpReceiveCallback(*this, event->value.asString);
        break;

    // TODO: OTA
    // TODO: system time
        
    default:
        break;
    }
}