#pragma once

#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "shell.h"

extern UART_HandleTypeDef huart1;
extern Shell shell;

void         shell_port_init(void);
void         shell_port_task(void);
unsigned int shell_port_get_tick(void);
