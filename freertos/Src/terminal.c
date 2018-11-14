/*
 * Copyright 2018 Tarkan Al-Kazily
 *
 * This file implements a Serial terminal task.
 *
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f1xx_hal.h"

#include "string.h"
#include "ctype.h"

#include "main.h"
#include "terminal.h"
#include "imu.h"

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

static char string_buffer[MAX_STRING_SIZE + 1];
static uint16_t string_length;
static char *pc;

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
  char *token = NULL;

  // Convert trailing \r to a null byte
  str[strlen(str) - 1] = '\0';
  // Ignore leading whitespace
  while (isspace(*ptr)) {
    ptr++;
  }
  token = strtok(ptr, " ");
  while (token && argc < max_args) {
    argv[argc] = token;
    argc++;
    token = strtok(NULL, " ");
  }

  return argc;
}

static int echo(char **argv, uint16_t argc) {
  for (uint8_t i = 1; i < argc; i++) {
    HAL_UART_Transmit(terminal_huart, (uint8_t *)argv[i], strlen(argv[i]), 100);
    if (i != argc - 1) {
      HAL_UART_Transmit(terminal_huart, (uint8_t *)" ", 1, 100);
    }
  }
  HAL_UART_Transmit(terminal_huart, (uint8_t *)"\r", 1, 100);
  return 0;
}

// PUBLIC FUNCTIONS

void terminal_print(char *str, uint16_t len) {
  HAL_UART_Transmit(terminal_huart, (uint8_t *) str, len, 1000);
}

void TerminalTask(void *argument) {

  terminal_queue_handle = xQueueCreateStatic(TERMINAL_QUEUE_LENGTH, TERMINAL_QUEUE_SIZE, terminal_queue_storage_buffer, &terminal_queue_buffer);

  do {
    // Get string from user over interrupt
    string_length = 0;
    string_buffer[MAX_STRING_SIZE] = '\0';
    HAL_UART_Receive_IT(terminal_huart, (uint8_t *)string_buffer, 1);

    char *str;
    BaseType_t ret = xQueueReceive(terminal_queue_handle, &str, portMAX_DELAY);
    if (!ret) {
      HAL_UART_Transmit(terminal_huart, (uint8_t *)"Error!\r", 7, 100);
    } else {
      HAL_UART_Transmit(terminal_huart, (uint8_t *)"\r", 1, 100);
      // str now points to our string
      str[string_length] = '\0';
      RunCommand(str, string_length);
    }
  } while (1);
}

int RunCommand(char *str, uint16_t len) {
  char *argv[16];
  int argc = tokenize(str, len, argv, 16);
  if (strncmp("echo", argv[0], strlen(argv[0])) == 0) {
    return echo(argv, argc);
  } else if (strncmp("imu", argv[0], strlen(argv[0])) == 0) {
    return imu_print(argv, argc);
  }
  HAL_UART_Transmit(terminal_huart, (uint8_t *)argv[0], strlen(argv[0]), 100);
  HAL_UART_Transmit(terminal_huart, (uint8_t *)error_msg, strlen(error_msg), 100);
  return 1;
}

void TerminalRxCallback() {
  LED_GPIO_Port->ODR ^= LED_Pin;
  string_length += 1;
  static char c;
  c = string_length < MAX_STRING_SIZE ? string_buffer[string_length - 1] : 0;
  if (c == '\r' && string_length == 1) {
    // Ignore 0 length string
    string_length = 0;
    HAL_UART_Receive_IT(terminal_huart, (uint8_t *)string_buffer + string_length, 1);
  } else if (string_length == MAX_STRING_SIZE || c == '\r') {
    // Process string
    pc = string_buffer;
    xQueueSendToBackFromISR(terminal_queue_handle, &pc, NULL);
  } else {
    if (isPrintable(c)) {
      HAL_UART_Transmit_IT(terminal_huart, (uint8_t *)&c, 1);
    }
    HAL_UART_Receive_IT(terminal_huart, (uint8_t *)string_buffer + string_length, 1);
  }
}
