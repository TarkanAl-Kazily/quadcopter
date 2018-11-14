/*
 * Copyright 2018 Tarkan Al-Kazily
 *
 * This file declares variables and functions for a Serial terminal task.
 *
 */

#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f1xx_hal.h"

// PUBLIC VARIABLES

extern UART_HandleTypeDef *terminal_huart;

#define TERMINAL_STACK_SIZE 2024
extern TaskHandle_t terminal_task_handle;
extern StackType_t terminal_stack_buffer[];
extern StaticTask_t terminal_task_buffer;

#define TERMINAL_QUEUE_LENGTH 4
#define TERMINAL_QUEUE_SIZE sizeof(char *)
extern QueueHandle_t terminal_queue_handle;
extern uint8_t terminal_queue_storage_buffer[];
extern StaticQueue_t terminal_queue_buffer;


// FUNCTIONS

void TerminalTask(void *argument);
void TerminalRxCallback();
int RunCommand(char *str, uint16_t len);

// Used to abstract the printing functionality so it is callable from other files.
void terminal_print(char *str, uint16_t len);

#endif // _TERMINAL_H_
