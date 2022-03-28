#pragma once

// #define TUYA_DEBUG

#ifdef TUYA_DEBUG
    #include <HardwareSerial.h>
    #define DEBUG_TUYA(fmt, ...) Serial.printf_P((PGM_P)PSTR(fmt "\r\n"), ## __VA_ARGS__ )
#else
	#define DEBUG_TUYA(...)	
#endif

