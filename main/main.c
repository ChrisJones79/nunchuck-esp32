#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nunchuck.h"


nunchuck_data_t data;

void app_main(void)
{
    init_nunchuck();
    ESP_LOGI("main", "nunchuck initialized");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("Timer: %lld μs\n", esp_timer_get_time());
        printf("%d, %d, %d, %d, %d, %d, %d\n",
               data.accelX, data.accelY, data.accelZ,
               data.analogX, data.analogY,
               data.buttonC, data.buttonZ);
    }
}