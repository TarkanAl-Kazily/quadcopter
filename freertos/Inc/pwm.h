/*
 * Copyright 2018 Tarkan Al-Kazily
 *
 * This file declares the pwm helper code.
 *
 */

#ifndef _PWM_H_
#define _PWM_H_

#include "stm32f1xx_hal.h"
#include "main.h"

void pwm_init(TIM_HandleTypeDef *htim);
void pwm_set_duty(uint16_t duty, uint32_t channel);
void pwm_set_throttle(uint16_t percent, uint32_t channel);
void pwm_calibrate();

#endif // _PWM_H_
