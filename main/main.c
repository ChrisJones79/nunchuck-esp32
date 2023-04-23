#include <stdio.h>
#include "nunchuck.h"

nunchuck_data_t data;


void app_main(void)
{
    init_nunchuck();
    ESP_LOGI("main", "nunchuck initialized");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        // printf("%d, %d, %d, %d, %d, %d, %d\n",
        //        data.accelX, data.accelY, data.accelZ,
        //        data.analogX, data.analogY,
        //        data.buttonC, data.buttonZ);
    }
}