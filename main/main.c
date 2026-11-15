#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

void app_main(void)
{
    printf("The Vessel: ESP32 I2C PWM initialized.\n");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
