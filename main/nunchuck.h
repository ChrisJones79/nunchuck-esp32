#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c.h"

#define I2C_FCLK                    100000
#define I2C_NUM                     0
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_TIMEOUT_MS              1000

#define I2C_NUNCHCUCK_ADDR 0x52
#define I2C_NUNCHUCK_RX 6

#define NINJA_STACK_SIZE 1980
const TickType_t xFrequency = 10;

typedef struct nunchuck_data{
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