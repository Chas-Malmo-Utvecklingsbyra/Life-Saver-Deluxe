#ifndef GUI_H
#define GUI_H

#include "internet/internet.h"

void lvgl_task(void *arg);

void GUI_Update_Network_Status(network_state_t state);

#endif