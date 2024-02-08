#include "TuyaDebug.h"
#include <sys/time.h>

#include "mbedtls/platform.h"
#include "mbedtls/cipher.h"
#include "mbedtls/md.h"

int mbedtls_message_digest_hmac(mbedtls_md_type_t md_type,
                                const uint8_t* key, size_t keylen,
                                const uint8_t* input, size_t ilen, 
                                uint8_t* digest)
    {
    if (key == NULL || keylen == 0 || input == NULL || ilen == 0 || digest == NULL)
        return -1;

    mbedtls_md_context_t md_ctx;
    mbedtls_md_init(&md_ctx);
    int ret = mbedtls_md_setup(&md_ctx, mbedtls_md_info_from_type(md_type), 1);
    if (ret != 0) {
        DEBUG_TUYA("mbedtls_md_setup() returned -0x%04x\n", -ret);
        goto exit;
    }

    mbedtls_md_hmac_starts(&md_ctx, key, keylen);
    mbedtls_md_hmac_update(&md_ctx, input, ilen);
    mbedtls_md_hmac_finish(&md_ctx, digest);

exit:
    mbedtls_md_free(&md_ctx);
    return ret;
}

int tuya_mqtt_auth_signature_calculate(const char* deviceId, const char* deviceSecret,
											  char* clientID, char* username, char* password) {

    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint32_t timestamp = (uint32_t)tv.tv_sec;

    /* client ID */
    sprintf(username, "%s|signMethod=hmacSha256,timestamp=%d,secureMode=1,accessType=1", deviceId, timestamp);
    DEBUG_TUYA("username:%s", username);

    /* username */
    sprintf(clientID, "tuyalink_%s", deviceId);
    DEBUG_TUYA("clientID:%s", clientID);

    /* password */
    int i = 0;
    char password_stuff[255];
    uint8_t digest[32];
    size_t slen = sprintf(password_stuff, "deviceId=%s,timestamp=%d,secureMode=1,accessType=1", deviceId, timestamp);
    mbedtls_message_digest_hmac(MBEDTLS_MD_SHA256, (const uint8_t*)deviceSecret, 16, (const uint8_t*)password_stuff, slen, digest);
    for (i = 0; i < 32; i++) {
        sprintf(password + 2*i, "%02x", digest[i]);
    }
    DEBUG_TUYA("password:%s", password);

    return 0;
}