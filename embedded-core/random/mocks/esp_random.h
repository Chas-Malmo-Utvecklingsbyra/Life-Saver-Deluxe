#ifndef ESP_RANDOM_H
#define ESP_RANDOM_H

#include <stddef.h>
#include <stdint.h>

void esp_fill_random(void *buf, size_t len);

#endif