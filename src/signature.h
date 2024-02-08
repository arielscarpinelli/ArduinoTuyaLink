#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int tuya_mqtt_auth_signature_calculate(const char* deviceId, const char* deviceSecret,
											  char* clientID, char* username, char* password);


#ifdef __cplusplus
}
#endif
