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

#define TERMINAL_STACK_SIZE 2024
TaskHandle_t terminal_task_handle;
StackType_t terminal_stack_buffer[TERMINAL_STACK_SIZE];
StaticTask_t terminal_task_buffer;

#define TERMINAL_QUEUE_LENGTH 4
#define TERMINAL_QUEUE_SIZE sizeof(char *)
QueueHandle_t terminal_queue_handle;
uint8_t terminal_queue_storage_buffer[TERMINAL_QUEUE_LENGTH * TERMINAL_QUEUE_SIZE];
StaticQueue_t terminal_queue_buffer;

// PRIVATE VARIABLES

#define MAX_STRING_SIZE 64

static char string_buffer[MAX_STRING_SIZE + 1];
static uint16_t string_length;
static char *pc;

static char *error_msg = ": command not recognized.\r";
static char *not_yet_impl_msg = "Not yet implemented.\r";
static char *initialized_msg = "\r\rInitialized.\r";

// HELPER FUNCTIONS

// Returns 1 if the character is printable by our standards. 0 otherwise.
static int isPrintable(char c) {
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
    return 1;
  } else if (c == ' ') {
    return 1;
  } else if (c >= '0' && c <= '9') {
    return 1;
  }
  return 0;
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

// The command of Bash fame
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

void terminal_init(UART_HandleTypeDef *huart) {
  // Initializes the huart handle
  terminal_huart = huart;

  // Starts the terminal task
  terminal_task_handle = xTaskCreateStatic(TerminalTask, "TerminalTask", TERMINAL_STACK_SIZE, NULL, 0, terminal_stack_buffer, &terminal_task_buffer);
}

void TerminalTask(void *argument) {

  terminal_queue_handle = xQueueCreateStatic(TERMINAL_QUEUE_LENGTH, TERMINAL_QUEUE_SIZE, terminal_queue_storage_buffer, &terminal_queue_buffer);
  HAL_UART_Transmit(terminal_huart, (uint8_t *)initialized_msg, strlen(initialized_msg), 100);

  do {
    // Get string from user over interrupt
    string_length = 0;
    string_buffer[MAX_STRING_SIZE] = '\0';
    HAL_UART_Receive_IT(terminal_huart, (uint8_t *)string_buffer, 1);

    char *str;
    BaseType_t ret = xQueueReceive(terminal_queue_handle, &str, portMAX_DELAY);
    if (!ret) {
      // Shouldn't happen, since we have an infinite wait time
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
  // Max 16 arguments
  char *argv[16];
  int argc = tokenize(str, len, argv, 16);
  // Run the desired command
  if (strncmp("echo", argv[0], strlen(argv[0])) == 0) {
    return echo(argv, argc);
  } else if (strncmp("imu", argv[0], strlen(argv[0])) == 0) {
    return imu_print(argv, argc);
  }
  // Handle no command found
  HAL_UART_Transmit(terminal_huart, (uint8_t *)argv[0], strlen(argv[0]), 100);
  HAL_UART_Transmit(terminal_huart, (uint8_t *)error_msg, strlen(error_msg), 100);
  return 1;
}

void TerminalRxCallback() {
  string_length += 1;
  char c = string_length < MAX_STRING_SIZE ? string_buffer[string_length - 1] : 0;
  if (c == '\r' && string_length == 1) {
    // Ignore 0 length string
    string_length = 0;
    HAL_UART_Receive_IT(terminal_huart, (uint8_t *)string_buffer + string_length, 1);
  } else if (string_length == MAX_STRING_SIZE || c == '\r') {
    // Process string and send it to the task.
    pc = string_buffer;
    xQueueSendToBackFromISR(terminal_queue_handle, &pc, NULL);
  } else {
    if (isPrintable(c)) {
      // Echo the character
      HAL_UART_Transmit_IT(terminal_huart, (uint8_t *)&string_buffer[string_length - 1], 1);
    } else {
      // Ignore bad characters
      string_length -= 1;
    }
    // Enable receiving another character
    HAL_UART_Receive_IT(terminal_huart, (uint8_t *)string_buffer + string_length, 1);
  }
}
