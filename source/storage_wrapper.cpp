#include "tuya_error_code.h"
#include "storage_interface.h"
#include "TuyaDebug.h"

int local_storage_set(const char* key, const uint8_t* buffer, size_t length)
{
    if (NULL == key || NULL == buffer || NULL == length) {
        return OPRT_INVALID_PARM;
    }

    DEBUG_TUYA("local_storage_set key %s", key);

    // return OPRT_COM_ERROR;
    return OPRT_OK;
}

int local_storage_get(const char* key, uint8_t* buffer, size_t* length)
{
    if (NULL == key || NULL == buffer || NULL == length) {
        return OPRT_INVALID_PARM;
    }

    return OPRT_COM_ERROR;
}

int local_storage_del(const char* key)
{
    return OPRT_OK;
}
