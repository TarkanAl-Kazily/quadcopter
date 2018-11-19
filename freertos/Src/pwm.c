/*
 * Copyright 2018 Tarkan Al-Kazily
 *
 * This file defines the pwm helper code.
 *
 * This will be used for controlling the ESC's
 *
 */

#include "stm32f1xx_hal.h"
#include "main.h"
#include "pwm.h"

// PRIVATE VARIABLES

TIM_HandleTypeDef *pwm_htim;

// PUBLIC FUNCTIONS

void pwm_init(TIM_HandleTypeDef *htim) {
  pwm_htim = htim;
  // Initialize PWM duty cycles.
  pwm_set_duty(300, TIM_CHANNEL_1);
  pwm_set_duty(0, TIM_CHANNEL_2);
  pwm_set_duty(0, TIM_CHANNEL_3);
  pwm_set_duty(0, TIM_CHANNEL_4);
  // Start PWM
  HAL_TIM_PWM_Start(pwm_htim, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(pwm_htim, TIM_CHANNEL_2);
}

void pwm_set_duty(uint16_t duty, uint32_t channel) {
  if (channel == TIM_CHANNEL_1) {
    pwm_htim->Instance->CCR1 = duty;
  }
  if (channel == TIM_CHANNEL_2) {
    pwm_htim->Instance->CCR2 = duty;
  }
  if (channel == TIM_CHANNEL_3) {
    pwm_htim->Instance->CCR3 = duty;
  }
  if (channel == TIM_CHANNEL_4) {
    pwm_htim->Instance->CCR4 = duty;
  }
}
