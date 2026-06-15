#ifndef MOCK_DEPS_H
#define MOCK_DEPS_H

#include "i2c_master.h"

extern esp_err_t mock_i2c_new_master_bus_result;

extern int mock_i2c_new_master_bus_call_count;

extern i2c_master_bus_handle_t mock_bus_handle;

#endif