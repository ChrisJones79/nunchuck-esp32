#pragma once

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

typedef struct nunchuck_data{
    int analogX;
    int analogY;

    int accelX;
    int accelY;
    int accelZ;

    bool buttonC;
    bool buttonZ;   

} nunchuck_data_t;

esp_err_t init_i2c(void);

void init_nunchuck(void);

nunchuck_data_t nunchuck_update(void);

void _nunchuck_sendByte_to_addr(uint8_t addr, uint8_t data);

void send_byte(uint8_t data);