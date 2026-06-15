#include "random.h"

#include <stdio.h>

#ifdef UNIT_TEST

#include "mocks/mock_deps.h"

#else

#include <esp_random.h>

#endif

void Random_Generate_UUID_v4(char *uuid_buffer) 
{
    uint8_t uuid[16];
    esp_fill_random(uuid, sizeof(uuid));

    uuid[6] = (uuid[6] & 0x0F) | 0x40;
    uuid[8] = (uuid[8] & 0x3F) | 0x80;

    snprintf(uuid_buffer, RANDOM_MAX_UUID_V4_LENGTH,
            "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid[0], uuid[1], uuid[2], uuid[3],
            uuid[4], uuid[5],
            uuid[6], uuid[7],
            uuid[8], uuid[9],
            uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
}