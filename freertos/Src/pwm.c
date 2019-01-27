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

#define MAX_THROTTLE 2000 // 10% of 20000
#define MIN_THROTTLE 1000 // 5%  of 20000

// PUBLIC FUNCTIONS

void pwm_init(TIM_HandleTypeDef *htim) {
  pwm_htim = htim;
  // Initialize PWM duty cycles.
  pwm_set_duty(0, TIM_CHANNEL_1);
  pwm_set_duty(0, TIM_CHANNEL_2);
  pwm_set_duty(0, TIM_CHANNEL_3);
  pwm_set_duty(0, TIM_CHANNEL_4);
  // Start PWM
  HAL_TIM_PWM_Start(pwm_htim, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(pwm_htim, TIM_CHANNEL_2);

  HAL_Delay(2000);
  pwm_calibrate();
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

void pwm_set_throttle(uint16_t percent, uint32_t channel) {
  if (percent > 100) {
    percent = 100;
  }
  uint16_t duty = MIN_THROTTLE + (((MAX_THROTTLE - MIN_THROTTLE) * percent) / 100);
  pwm_set_duty(duty, channel);
}

void pwm_calibrate() {
  pwm_set_throttle(100, TIM_CHANNEL_1);
  pwm_set_throttle(100, TIM_CHANNEL_2);
  pwm_set_throttle(100, TIM_CHANNEL_3);
  pwm_set_throttle(100, TIM_CHANNEL_4);
  HAL_Delay(2000);
  pwm_set_throttle(0, TIM_CHANNEL_1);
  pwm_set_throttle(0, TIM_CHANNEL_2);
  pwm_set_throttle(0, TIM_CHANNEL_3);
  pwm_set_throttle(0, TIM_CHANNEL_4);
}
