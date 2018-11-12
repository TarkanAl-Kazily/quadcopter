/*
 * Copyright 2018 Tarkan Al-Kazily
 *
 * This file defines the imu code.
 * This code is for using the MPU 6050 over I2C.
 *
 */

#include "imu.h"
#include "main.h"

// PUBLIC VARIABLES

uint16_t imu_accel_x;
uint16_t imu_accel_y;
uint16_t imu_accel_z;

// PRIVATE VARIABLES

#define IMU_ADDRESS           0x68
I2C_HandleTypeDef *imu_i2c_h;

#define REG_READ              0x00
#define REG_WRITE             0x80

// Registers
#define IMU_REG_CONFIG          26
#define IMU_REG_GYRO_CONFIG     27
#define IMU_REG_ACCEL_CONFIG    28

#define IMU_REG_INT_PIN_CFG     55
#define IMU_REG_INT_ENABLE      56
#define IMU_REG_INT_STATUS      57

#define IMU_REG_ACCEL_XOUT_H    59
#define IMU_REG_ACCEL_XOUT_L    60
#define IMU_REG_ACCEL_YOUT_H    61
#define IMU_REG_ACCEL_YOUT_L    62
#define IMU_REG_ACCEL_ZOUT_H    63
#define IMU_REG_ACCEL_ZOUT_L    64

#define IMU_REG_TEMP_OUT_H      65
#define IMU_REG_TEMP_OUT_L      66

#define IMU_REG_GYRO_XOUT_H     67
#define IMU_REG_GYRO_XOUT_L     68
#define IMU_REG_GYRO_YOUT_H     69
#define IMU_REG_GYRO_YOUT_L     70
#define IMU_REG_GYRO_ZOUT_H     71
#define IMU_REG_GYRO_ZOUT_L     72

// HELPER FUNCTIONS

// Writes len bytes to addr, starting at reg. Returns 0 on success.
static int reg_write(uint16_t addr, uint8_t reg, uint8_t *data, uint16_t len) {
  reg |= REG_WRITE;
  HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(imu_i2c_h, addr, &reg, 1, 1000);
  ret = HAL_I2C_Master_Transmit(imu_i2c_h, addr, data, len, 1000);

  return 0;
}

// Reads len bytes from addr, starting at reg. Returns 0 on success.
static int reg_read(uint16_t addr, uint8_t reg, uint8_t *data, uint16_t len) {
  reg |= REG_READ;
  HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(imu_i2c_h, addr, &reg, 1, 1000);
  ret = HAL_I2C_Master_Receive(imu_i2c_h, addr, data, len, 1000);

  return 0;
}

// PUBLIC FUNCTIONS

int imu_init(I2C_HandleTypeDef *i2c_h) {
  imu_i2c_h = i2c_h;

  // Start a calibration

  // Start a calculation

  uint8_t buf[6];
  buf[0] = 0x00;
  // Set ranges to be 0
  reg_write(IMU_ADDRESS, IMU_REG_ACCEL_CONFIG, buf, 1);

  while (1) {
    // Read back x, y, z accelerometer data
    reg_read(IMU_ADDRESS, IMU_REG_ACCEL_XOUT_H, buf, 6);
    imu_accel_x = (buf[0] << 8) | buf[1];
    imu_accel_y = (buf[2] << 8) | buf[3];
    imu_accel_z = (buf[4] << 8) | buf[5];
    HAL_Delay(500);
  }
  return 0;
}
