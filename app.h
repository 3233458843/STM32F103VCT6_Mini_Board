#pragma once

#include "main.h"

/* 由 shell 的 oled 命令调用，在信息面板上显示一条消息 */
void app_set_message(const char *msg);

/* LED 手动控制：接管后停止 UI 心跳闪烁 */
void app_led_manual(void);
void app_led_auto(void);
