/*
 * Copyright 2018 Tarkan Al-Kazily
 *
 * This file defines the imu code.
 * This code is for using the MPU 6050 over I2C.
 *
 */

#include "FreeRTOS.h"
#include "string.h"
#include "stdio.h"

#include "main.h"
#include "imu.h"
#include "terminal.h"

// PUBLIC VARIABLES

imu_accel_t imu_accel_raw;
imu_accel_t imu_accel_cal;
imu_accel_t imu_accel_avg;

// PRIVATE VARIABLES

#define IMU_STACK_SIZE 1024

TaskHandle_t imu_task_handle;
StackType_t imu_stack_buffer[IMU_STACK_SIZE];
StaticTask_t imu_task_buffer;

// Must shift address left one bit for HAL APIs
#define IMU_ADDRESS           (0x68 << 1)
I2C_HandleTypeDef *imu_i2c_h;

// Registers
#define IMU_REG_CONFIG               26
#define IMU_REG_GYRO_CONFIG          27
#define IMU_REG_ACCEL_CONFIG         28
#define IMU_REG_ACCEL_CONFIG_2       29

#define IMU_REG_INT_PIN_CFG          55
#define IMU_REG_INT_ENABLE           56
#define IMU_REG_INT_STATUS           57

#define IMU_REG_ACCEL_XOUT_H         59
#define IMU_REG_ACCEL_XOUT_L         60
#define IMU_REG_ACCEL_YOUT_H         61
#define IMU_REG_ACCEL_YOUT_L         62
#define IMU_REG_ACCEL_ZOUT_H         63
#define IMU_REG_ACCEL_ZOUT_L         64

#define IMU_REG_TEMP_OUT_H           65
#define IMU_REG_TEMP_OUT_L           66

#define IMU_REG_GYRO_XOUT_H          67
#define IMU_REG_GYRO_XOUT_L          68
#define IMU_REG_GYRO_YOUT_H          69
#define IMU_REG_GYRO_YOUT_L          70
#define IMU_REG_GYRO_ZOUT_H          71
#define IMU_REG_GYRO_ZOUT_L          72

#define IMU_REG_SIGNAL_PATH_RESET   104

#define IMU_REG_USER_CTRL           106
#define IMU_REG_PWR_MGMT_1          107
#define IMU_REG_PWR_MGMT_2          108

#define IMU_REG_WHO_AM_I            117

#define IMU_DATA_SIZE 10
imu_accel_t imu_accel_data[IMU_DATA_SIZE];

// HELPER FUNCTIONS

// Writes len bytes to addr, starting at reg. Returns 0 on success.
static int reg_write(uint16_t addr, uint8_t reg, uint8_t *data, uint16_t len) {
  return HAL_I2C_Mem_Write(imu_i2c_h, addr, reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

// Reads len bytes from addr, starting at reg. Returns 0 on success.
static int reg_read(uint16_t addr, uint8_t reg, uint8_t *data, uint16_t len) {
  return HAL_I2C_Mem_Read(imu_i2c_h, addr, reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

static void imu_accel_read(imu_accel_t *acc) {
  uint8_t buf[6];
  // Read x, y, z accelerometer data
  reg_read(IMU_ADDRESS, IMU_REG_ACCEL_XOUT_H, buf, 6);
  acc->x = (buf[0] << 8) | buf[1];
  acc->y = (buf[2] << 8) | buf[3];
  acc->z = (buf[4] << 8) | buf[5];
}

// Initializes imu_accel_cal to be an averaged calibration of the accelerometer readings
static void imu_calibrate(uint16_t iterations) {
  memset(&imu_accel_cal, 0, sizeof(imu_accel_cal));
  imu_accel_t imu_accel_tmp;
  for (uint16_t i = 0; i < iterations; i++) {
    imu_accel_read(&imu_accel_tmp);
    imu_accel_cal.x += (imu_accel_tmp.x / iterations);
    imu_accel_cal.y += (imu_accel_tmp.y / iterations);
    imu_accel_cal.z += (imu_accel_tmp.z / iterations);

    vTaskDelay(5);
  }
}

// Initializes the main MPU
static uint8_t imu_device_init() {
  uint8_t data;
  // Reset the internal registers
  data = 0x80;
  reg_write(IMU_ADDRESS, IMU_REG_PWR_MGMT_1, &data, 1);
  HAL_Delay(100);
  // Reset gyro/accel/temp digital signal path
  data = 0x07;
  reg_write(IMU_ADDRESS, IMU_REG_SIGNAL_PATH_RESET, &data, 1);
  HAL_Delay(100);

  uint8_t IMU_REG_Init_Data[7][2] = {
    { IMU_REG_PWR_MGMT_1,     0x03 }, // Sets Clock Source to the Z axis gyro
    { IMU_REG_PWR_MGMT_2,     0x00 }, // all enable
    { IMU_REG_CONFIG,         0x04 }, // set frequency of samples
    { IMU_REG_GYRO_CONFIG,    0x18 }, // +-2000dps
    { IMU_REG_ACCEL_CONFIG,   0x10 }, // +-8G
    { IMU_REG_ACCEL_CONFIG_2, 0x04 }, // acc bandwidth 20Hz
    { IMU_REG_USER_CTRL,      0x20 }, // Enable the I2C Master I/F module
    // pins ES_DA and ES_SCL are isolated from 
    // pins SDA/SDI and SCL/SCLK.
  };
  uint8_t i = 0;
  for (i = 0; i < 7; i++)
  {
    reg_write(IMU_ADDRESS, IMU_REG_Init_Data[i][0], &IMU_REG_Init_Data[i][1], 1);
    HAL_Delay(1);
  }

  /*
  // Reset ist
  HAL_GPIO_WritePin(IST_SET_GPIO_Port, IST_SET_Pin, GPIO_PIN_RESET);
  HAL_Delay(2000);
  HAL_GPIO_WritePin(IST_SET_GPIO_Port, IST_SET_Pin, GPIO_PIN_SET);
  HAL_Delay(2000);
  */
  imu_calibrate(150);
  return 0;
}

// Called once every time new imu data arrives. Updates the average IMU value
static void imu_update_avg() {
  static uint8_t next = 0;

  // Classic moving average computation
  imu_accel_t imu_old = imu_accel_data[next];
  imu_accel_data[next] = imu_accel_raw;
  imu_accel_avg.x += (imu_accel_raw.x - imu_old.x) / IMU_DATA_SIZE;
  imu_accel_avg.y += (imu_accel_raw.y - imu_old.y) / IMU_DATA_SIZE;
  imu_accel_avg.z += (imu_accel_raw.z - imu_old.z) / IMU_DATA_SIZE;

  next = (next + 1) % IMU_DATA_SIZE;
}

// PUBLIC FUNCTIONS

int imu_init(I2C_HandleTypeDef *i2c_h) {
  imu_i2c_h = i2c_h;

  // Initialize the IMU
  imu_device_init();

  // Start a calculation
  imu_task_handle = xTaskCreateStatic(ImuTask, "ImuTask", IMU_STACK_SIZE, NULL, 0, imu_stack_buffer, &imu_task_buffer);
  return 0;
}

void ImuTask(void *argument) {
  while (1) {
    // Read x, y, z accelerometer data
    imu_accel_read(&imu_accel_raw);
    imu_update_avg();
    vTaskDelay(5);
  }
}

int imu_print(char **argv, uint16_t argc) {
  uint32_t iterations = 1;
  uint32_t delay = 500;
  char buf[100];
  if (argc > 1) {
    sscanf(argv[1], "%lu", &iterations);
  }
  if (argc > 2) {
    sscanf(argv[2], "%lu", &delay);
  }

  snprintf(buf, 100, "Offset: %d, %d, %d\n", imu_accel_cal.x,
                                             imu_accel_cal.y,
                                             imu_accel_cal.z);
  terminal_print(buf, strlen(buf));
  vTaskDelay(10);
  while (iterations-- > 0) {
    snprintf(buf, 100, "%d, %d, %d\n", imu_accel_avg.x - imu_accel_cal.x,
                                       imu_accel_avg.y - imu_accel_cal.y,
                                       imu_accel_avg.z - imu_accel_cal.z);
    terminal_print(buf, strlen(buf));
    vTaskDelay(delay);
  }

  return 0;
}
