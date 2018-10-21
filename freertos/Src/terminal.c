/*
 * Copyright 2018 Tarkan Al-Kazily
 *
 * This file implements a Serial terminal task.
 *
 */

#include "terminal.h"
#include "string.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f1xx_hal.h"

// PUBLIC VARIABLES

UART_HandleTypeDef *terminal_huart;

TaskHandle_t terminal_task_handle;
StackType_t terminal_stack_buffer[TERMINAL_STACK_SIZE];
StaticTask_t terminal_task_buffer;

QueueHandle_t terminal_queue_handle;
uint8_t terminal_queue_storage_buffer[TERMINAL_QUEUE_LENGTH * TERMINAL_QUEUE_SIZE];
StaticQueue_t terminal_queue_buffer;

// PRIVATE VARIABLES

#define MAX_STRING_SIZE 64

static char string_buffer[MAX_STRING_SIZE];
static uint16_t string_length;

void TerminalTask(void *argument) {

  string_length = 0;
  HAL_UART_Receive_IT(terminal_huart, (uint8_t *)string_buffer, 1);

  while (1) {
    char *str;
    BaseType_t ret = xQueueReceive(terminal_queue_handle, &str, portMAX_DELAY);
    if (!ret) {
      HAL_UART_Transmit(terminal_huart, "bad\n", 4, 1000);
    } else {
      for (uint16_t i = 0; i < MAX_STRING_SIZE; i++) {
        if (str[i] == '\n') {
          str[i] = 0;
        }
      }
      HAL_UART_Transmit(terminal_huart, (uint8_t *)str, strlen(str), 1000);
    }

    string_length = 0;
    HAL_UART_Receive_IT(terminal_huart, (uint8_t *)string_buffer, 1);
  }
}

void TerminalRxCallback() {
  LED_GPIO_Port->ODR ^= LED_Pin;
  string_length += 1;
  if (string_length == MAX_STRING_SIZE || string_buffer[string_length - 1] == '\n') {
    char *pc = string_buffer;
    xQueueSendToBackFromISR(terminal_queue_handle, &pc, NULL);
  } else {
    HAL_UART_Receive_IT(terminal_huart, (uint8_t *)string_buffer + string_length, 1);
  }
}
