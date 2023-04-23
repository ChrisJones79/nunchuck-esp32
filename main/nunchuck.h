#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c.h"

typedef struct nunchuck_data
{
    int analogX;
    int analogY;

    int accelX;
    int accelY;
    int accelZ;

    bool buttonC;
    bool buttonZ;

} nunchuck_data_t;

void init_nunchuck(void);

nunchuck_data_t nunchuck_update(void);