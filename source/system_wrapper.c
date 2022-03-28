#include <stdlib.h>
#include <sys/time.h>

#include "system_interface.h"

void* system_malloc(size_t n)
{
    return malloc(n);
}

void* system_calloc(size_t n, size_t size)
{
    return calloc(n, size);
}

void  system_free(void *ptr)
{
    free(ptr);
}

uint32_t system_ticks(void)
{
    return millis();
}

uint32_t system_timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t)tv.tv_sec;
}

void system_sleep( uint32_t time_ms )
{
    delay(time_ms);
}

uint32_t system_random(void)
{
    return (uint32_t)rand();
}