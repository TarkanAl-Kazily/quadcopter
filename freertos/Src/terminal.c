/*
 * Copyright 2018 Tarkan Al-Kazily
 *
 * This file implements a Serial terminal task.
 *
 */

#include "terminal.h"
#include "string.h"
#include "ctype.h"
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

static char *error_msg = ": command not recognized.\n";
static char *not_yet_impl_msg = "Not yet implemented.\n";

// HELPER FUNCTIONS

static int isPrintable(char c) {
  return isprint(c) || c == ' ';
}

// Tokenizes a string into argv style arguments. Returns the number of arguments found.
static uint16_t tokenize(char *str, uint16_t len, char **argv, uint16_t max_args) {
  char *ptr = str;
  uint16_t argc = 0;
  while (len != 0 && argc < max_args) {
    argv[argc++] = ptr;
    while (len-- > 0 && !isspace(*ptr)) {
      ptr++;
    }
    if (len == 0) {
      break;
    }
    *ptr = '\0';
    ptr++;
  }
  return argc;
}

static int echo(char **argv, uint16_t argc) {
  for (uint8_t i = 1; i < argc - 1; i++) {
    argv[i][strlen(argv[i])] = ' ';
  }
  taskENTER_CRITICAL();
  HAL_UART_Transmit(terminal_huart, (uint8_t *)argv[1], strlen(argv[1]), 1000);
  HAL_UART_Transmit(terminal_huart, (uint8_t *)"\n", 1, 100);
  taskEXIT_CRITICAL();
  return 0;
}

// PUBLIC FUNCTIONS

void TerminalTask(void *argument) {
  // Get string from user over interrupt
  string_length = 0;
  HAL_UART_Receive_IT(terminal_huart, (uint8_t *)string_buffer, 1);

  while (1) {
    char *str;
    BaseType_t ret = xQueueReceive(terminal_queue_handle, &str, portMAX_DELAY);
    if (!ret) {
      taskENTER_CRITICAL();
      HAL_UART_Transmit(terminal_huart, (uint8_t *)"Error!\n", 7, 100);
      taskEXIT_CRITICAL();
    } else {
      taskENTER_CRITICAL();
      HAL_UART_Transmit(terminal_huart, (uint8_t *)"\n", 1, 100);
      taskEXIT_CRITICAL();
      // str now points to our string
      if (str[string_length - 1] == '\t') {
        str[string_length - 1] = '\0';
        AutoCommand(str, string_length - 1);
      } else {
        str[string_length - 1] = '\0';
        RunCommand(str, string_length - 1);
      }
    }

    string_length = 0;
    HAL_UART_Receive_IT(terminal_huart, (uint8_t *)string_buffer, 1);
  }
}

int RunCommand(char *str, uint16_t len) {
  char *argv[16];
  int argc = tokenize(str, len, argv, 16);
  if (strncmp("echo", argv[0], strlen(argv[0])) == 0) {
    return echo(argv, argc);
  }
  taskENTER_CRITICAL();
  HAL_UART_Transmit(terminal_huart, (uint8_t *)argv[0], strlen(argv[0]), 100);
  HAL_UART_Transmit(terminal_huart, (uint8_t *)error_msg, strlen(error_msg), 100);
  taskEXIT_CRITICAL();
  return 1;
}

int AutoCommand(char *str, uint16_t len) {
  taskENTER_CRITICAL();
  HAL_UART_Transmit(terminal_huart, (uint8_t *)not_yet_impl_msg, strlen(not_yet_impl_msg), 100);
  taskEXIT_CRITICAL();
  return 1;
}

void TerminalRxCallback() {
  LED_GPIO_Port->ODR ^= LED_Pin;
  string_length += 1;
  char c = string_length < MAX_STRING_SIZE ? string_buffer[string_length - 1] : 0;
  if (string_length == MAX_STRING_SIZE || c == '\n' || c == '\t') {
    // Process string
    char *pc = string_buffer;
    xQueueSendToBackFromISR(terminal_queue_handle, &pc, NULL);
  } else {
    if (isPrintable(c)) {
      HAL_UART_Transmit_IT(terminal_huart, (uint8_t *)&c, 1);
    }
    HAL_UART_Receive_IT(terminal_huart, (uint8_t *)string_buffer + string_length, 1);
  }
}
