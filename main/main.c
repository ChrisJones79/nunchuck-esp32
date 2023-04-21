#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nunchuck.h"

#define NINJA_STACK_SIZE 1024

nunchuck_data_t data;

void createNunchuckTask(void);
void vUpdateNunchuckPeriodic(void *pvParameters);

void app_main(void)
{
    esp_err_t ret = init_i2c();

    if (ret != ESP_OK)
    {
        ESP_LOGE("I2c_init", "I2C init failed: %s\n", esp_err_to_name(ret));
    }

    init_nunchuck();
    ESP_LOGI("main", "nunchuck initialized");

    createNunchuckTask();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        // printf("Timer: %lld μs\n", esp_timer_get_time());
        printf("%d, %d, %d, %d, %d, %d, %d\n",
               data.accelX, data.accelY, data.accelZ,
               data.analogX, data.analogY,
               data.buttonC, data.buttonZ);
    }
}

/* Update the nunchuck data periodically */
void vUpdateNunchuckPeriodic(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 10;

    data = (nunchuck_data_t *)pvParameters;
    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
    UBaseType_t uxHighWaterMark;

    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    printf("High water mark: %d", uxHighWaterMark);

    for (;;)
    {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        data = nunchuck_update();

        /*
        Calling the function will have used some stack space, we would
        therefore now expect uxTaskGetStackHighWaterMark() to return a
        value lower than when it was called on entering the task.
         */
        uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        printf("High water mark after update: %d", uxHighWaterMark);
    }
}

/* Function that creates a task. */
void createNunchuckTask(void)
{
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
        vUpdateNunchuckPeriodic, /* Function that implements the task. */
        "NunchuckUpdate",        /* Text name for the task. */
        NINJA_STACK_SIZE,        /* Stack size in words, not bytes. */
        (void *)data,            /* Parameter passed into the task. */
        tskIDLE_PRIORITY,        /* Priority at which the task is created. */
        &xHandle);               /* Used to pass out the created task's handle. */

    if (xReturned == pdPASS)
    {
        /* The task was created.  Use the task's handle to delete the task. */
        vTaskDelete(xHandle);
    }
}