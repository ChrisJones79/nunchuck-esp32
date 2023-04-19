#include <stdio.h>
#include "esp_log.h"
#include "i2c0.h"

void app_main(void)
{
    esp_err_t ret = init_i2c();

    if (ret != ESP_OK)
    {
        ESP_LOGE("I2c_init", "I2C init failed: %s\n", esp_err_to_name(ret));
    }

    init_nunchuck();
    ESP_LOGI("main", "nunchuck initialized");

    nunchuck_data_t data; ///= nunchuck_update();

    vTaskDelay(pdMS_TO_TICKS(20));

    inline float mult(int rdg)
    {
        return rdg * 9.8 / 969;
    }

    float ax, ay, az;

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));

        data = nunchuck_update();

        ax = mult(data.accelX);
        ay = mult(data.accelY);
        az = mult(data.accelZ);

        printf("/*ChrisFinallyDidIt,%4.2f,%4.2f,%4.2f,%d,%d,%d,%d*/\n", ax, ay, az,
               data.analogX, data.analogY,
               data.buttonC, data.buttonZ);

    }
}
