/*
 * Copyright 2018 Tarkan Al-Kazily
 *
 * This file declares variables and functions for use with the IMU
 *
 * Primarily uses features for the MPU 6050
 *
 */

#ifndef _IMU_H_
#define _IMU_H_

#include "FreeRTOS.h"
#include "stm32f1xx_hal.h"

// PUBLIC VARIABLES


// FUNCTIONS

// Initializes the imu system. Returns 0 on success, 1 otherwise.
int imu_init(I2C_HandleTypeDef *i2c_h);

#endif // _IMU_H_
