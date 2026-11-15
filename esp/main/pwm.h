
#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include "esp_err.h"

esp_err_t pwm_init(void);

esp_err_t pwm_set_duty(uint8_t channel, float duty);

#endif
