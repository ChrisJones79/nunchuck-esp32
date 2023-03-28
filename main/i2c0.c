#include "i2c0.h"

static const i2c_mode_t I2C_mode = I2C_MODE_MASTER;
static const i2c_port_t I2C_port = I2C_NUM_0;

esp_err_t init_i2c(void)
{

    const i2c_config_t conf = {
        .mode = I2C_mode,
        .sda_io_num = 14,
        .scl_io_num = 13,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FCLK,
    };

    esp_err_t ret = i2c_param_config(I2C_port, &conf);

    if (ret != ESP_OK)
    {
        ESP_LOGE("I2C_params", "config failed: %s", esp_err_to_name(ret));
    }

    return i2c_driver_install(I2C_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void init_nunchuck(void)
{
    // uint8_t init_bytes[] = {0xf0, 0x55, 0xfb, 0x55, 0x00, 0x00};
    // _nunchuck_sendByte_to_addr(0xf0, 0x55);
    // vTaskDelay(10);
    _nunchuck_sendByte_to_addr(0xf0, 0x55);
    // vTaskDelay(10);
    _nunchuck_sendByte_to_addr(0xfb, 0x00);
    // vTaskDelay(20);

    send_byte(0xFA);
    // send_byte(0x00);
    /*
    Note that Sparfun reference shows, rather confusingly, that
    the order of sending the start-up bytes is like so, reverse from
    the bytes order in the _sendByte(data, addr) command. Sending addr, data.
    So the buffer would be sent idx1 then idx0.
    */

    nunchuck_update();
}

nunchuck_data_t nunchuck_update(void)
{
    uint8_t rx_buf[6] = {0};

    esp_err_t ret = ESP_OK;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, (I2C_NUNCHCUCK_ADDR << 1) | I2C_MASTER_READ, true);

    i2c_master_read(cmd, rx_buf, sizeof(rx_buf) / sizeof(uint8_t) - 1, I2C_MASTER_ACK);

    i2c_master_read(cmd, &rx_buf[5], 1, I2C_MASTER_LAST_NACK);

    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_port, cmd, pdTICKS_TO_MS(100));
    if (ret != ESP_OK)
    {
        ESP_LOGE("nunchuck_update", "i2c_master_cmd_begin fail: %s", esp_err_to_name(ret));
    }

    i2c_cmd_link_delete(cmd);

    nunchuck_data_t dat = {
        .analogX = rx_buf[0],
        .analogY = rx_buf[1],
        .accelX = (rx_buf[2] << 2) | ((rx_buf[5] >> 2) & 3),
        .accelY = (rx_buf[3] << 2) | ((rx_buf[5] >> 4) & 3),
        .accelZ = (rx_buf[4] << 2) | ((rx_buf[5] >> 6) & 3),
        .buttonC = ((rx_buf[5] >> 1) & 1),
        .buttonZ = ((rx_buf[5] >> 0) & 1)
    };

    send_byte(0x00);

    return dat;
}

void _nunchuck_sendByte_to_addr(uint8_t addr, uint8_t data)
{
    // uint8_t write_buf[] = {addr, data};
    esp_err_t ret = ESP_OK;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    
    i2c_master_write_byte(cmd, (I2C_NUNCHCUCK_ADDR << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, addr, true);
    
    i2c_master_write_byte(cmd, data, true);
    
    i2c_master_stop(cmd);
    
    ret = i2c_master_cmd_begin(I2C_port, cmd, pdMS_TO_TICKS(100));
    if (ret != ESP_OK)
    {
        ESP_LOGE("_nunchuck_sendByte_to_addr", "cmd_begin failed: %s", esp_err_to_name(ret));
    }

    i2c_cmd_link_delete(cmd);
}

void send_byte(uint8_t data)
{
    esp_err_t ret = ESP_OK;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);

    i2c_master_write_byte(cmd, (I2C_NUNCHCUCK_ADDR << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write(cmd, &data, 1, true);

    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_port, cmd, 20);
    if (ret != ESP_OK)
    {
        ESP_LOGE("send_byte", "fail: %s", esp_err_to_name(ret));
    }

    i2c_cmd_link_delete(cmd);

}