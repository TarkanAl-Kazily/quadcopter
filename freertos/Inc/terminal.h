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

// FUNCTIONS

void terminal_init(UART_HandleTypeDef *huart);
void TerminalTask(void *argument);
void TerminalRxCallback();
int RunCommand(char *str, uint16_t len);

// Used to abstract the printing functionality so it is callable from other files.
// Should only be called in the TerminalTask.
void terminal_print(char *str, uint16_t len);

#endif // _TERMINAL_H_
