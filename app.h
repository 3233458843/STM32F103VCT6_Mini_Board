#pragma once

#include "main.h"

/* 由 shell 的 oled 命令调用，在信息面板上显示一条消息 */
void app_set_message(const char *msg);
