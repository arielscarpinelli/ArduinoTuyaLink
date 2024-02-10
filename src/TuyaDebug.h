#pragma once

#if defined(CORE_DEBUG_LEVEL) || defined(DEBUG_ESP_PORT) || defined(TUYA_DEBUG)
    #include <HardwareSerial.h>
    #define DEBUG_TUYA(fmt, ...) Serial.printf_P((PGM_P)PSTR(fmt "\r\n"), ## __VA_ARGS__ )
#else
	#define DEBUG_TUYA(...)	
#endif

