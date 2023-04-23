#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"

#include "nunchuck.h"

#define I2C_FCLK 100000
#define I2C_NUM 0
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_TIMEOUT_MS 1000

#define I2C_NUNCHCUCK_ADDR 0x52
#define I2C_NUNCHUCK_RX 6

#define NINJA_STACK_SIZE 1980
const TickType_t xFrequency = 100;

static const i2c_mode_t I2C_mode = I2C_MODE_MASTER;
static const i2c_port_t I2C_port = I2C_NUM_0;
static TaskHandle_t xNunchuckTaskHandle = NULL;

extern nunchuck_data_t data;

static esp_err_t init_i2c(void);

static void _nunchuck_sendByte_to_addr(uint8_t addr, uint8_t data);

static void send_byte(uint8_t data);

static void createNunchuckTask(void);

static void vUpdateNunchuckPeriodic(void *pvParameters);

// static void destroyNunchuckTask(void);

/*
 * Initialize the I2C bus.
 * - Set the I2C mode to master
 * - Set the SDA and SCL pins
 * - Set the SDA and SCL pullup resistors
 * - Set the clock speed
 * - Install the I2C driver
 * - Return the result of the driver installation
 */

static esp_err_t init_i2c(void)
{

    const i2c_config_t conf = {
        .mode = I2C_mode,
        .sda_io_num = 4,
        .scl_io_num = 5,
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

/*
 * Initialize the nunchuck.
 *
 * The startup sequence is sent to the nunchuck, then the first update
 * is performed.
 */
void init_nunchuck(void)
{
    // Initialize the I2C bus from the nunchuck init function
    esp_err_t ret = init_i2c();

    if (ret != ESP_OK)
    {
        ESP_LOGE("init_nunchuck", "i2c_driver install fail %s", esp_err_to_name(ret));
    }
    _nunchuck_sendByte_to_addr(0xf0, 0x55);
    _nunchuck_sendByte_to_addr(0xfb, 0x00);

    send_byte(0xFA);

    nunchuck_update();

    createNunchuckTask();
}

/*
 * Update the nunchuck data.
 * - Send the update command
 * - Read the data from the nunchuck
 * - Return the data
 * - If the I2C transaction fails, print an error message
 * - If the I2C transaction succeeds, return the data
 *
 */
nunchuck_data_t nunchuck_update(void)
{
    uint8_t rx_buf[] = {0, 0, 0, 0, 0, 0};

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

    // Note that the buttons are inverted,
    // that is unpressed they are 1 and pressed 0
    nunchuck_data_t dat = {
        .analogX = rx_buf[0],
        .analogY = rx_buf[1],
        .accelX = (rx_buf[2] << 2) | ((rx_buf[5] >> 2) & 3),
        .accelY = (rx_buf[3] << 2) | ((rx_buf[5] >> 4) & 3),
        .accelZ = (rx_buf[4] << 2) | ((rx_buf[5] >> 6) & 3),
        .buttonC = ((rx_buf[5] >> 1) & 1),
        .buttonZ = ((rx_buf[5] >> 0) & 1)};

    send_byte(0x00);

    printf("%d, %d, %d, %d, %d, %d, %d\n",
           data.accelX, data.accelY, data.accelZ,
           data.analogX, data.analogY,
           data.buttonC, data.buttonZ);

    return dat;
}

/*
 * Send a byte to the nunchuck.
 * - Create a new I2C command
 * - Start the I2C transaction
 * - Write the nunchuck address and the write bit
 * - Write the register address to write to
 * - Write the data to
 * - Stop the I2C transaction
 * - Return the result of the I2C transaction
 * - If the I2C transaction fails, print an error message
 * - If the I2C transaction succeeds, return the result
 */

static void _nunchuck_sendByte_to_addr(uint8_t addr, uint8_t data)
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

static void send_byte(uint8_t data)
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

/* Update the nunchuck data periodically */
static void vUpdateNunchuckPeriodic(void *pvParameters)
{
    // Initialise the xLastWakeTime variable with the current time.
    TickType_t xLastWakeTime = xTaskGetTickCount();

    /* Inspect our own high water mark on entering the task. */
    // UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // printf("High water mark: %d\n", uxHighWaterMark);
    // long long int tstart;
    // long long int tend;

    for (;;)
    {
        // tstart = esp_timer_get_time();
        // Wait for the next cycle.
        // printf("Timer before delayUntil: %lld μs\n", tstart);

        // vTaskDelayUntil(&xLastWakeTime, xFrequency);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        // tend = esp_timer_get_time();
        // printf("Timer after delayUntil %lld with frequency set to %ld\n", tend, xFrequency);
        data = nunchuck_update();
        // printf("Timer after nunchuck_update: %lld μs\n", esp_timer_get_time());

        /*
        Calling the function will have used some stack space, we would
        therefore now expect uxTaskGetStackHighWaterMark() to return a
        value lower than when it was called on entering the task.
         */
        // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        // printf("High water mark after update: %d\n", uxHighWaterMark);
    }
}

/* Function that creates a task. */
static void createNunchuckTask(void)
{
    printf("Creating nunchuck task ... ");
    BaseType_t xReturned;

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
        vUpdateNunchuckPeriodic, /* Function that implements the task. */
        "NunchuckUpdate",        /* Text name for the task. */
        NINJA_STACK_SIZE,        /* Stack size in words, not bytes. */
        (void *)NULL,            /* Parameter passed into the task. */
        tskIDLE_PRIORITY,        /* Priority at which the task is created. */
        &xNunchuckTaskHandle);   /* Used to pass out the created task's handle. */

    if (xReturned == pdPASS)
    {
        printf("successfully\n");
    }
    else
    {
        printf("unsuccessfully\n");
    }
}

/*
 * This task takes a task handle and destroys the task.
 */
// static void destroyNunchuckTask(void)
// {
//     vTaskDelete(xNunchuckTaskHandle);
// }