#include "mock_deps.h"

#include <string.h>

uint8_t mock_random_bytes[16];

void esp_fill_random(void *buf, size_t len)
{
    memcpy(buf, mock_random_bytes, len);
}