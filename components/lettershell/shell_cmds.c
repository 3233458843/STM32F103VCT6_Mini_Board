#include "shell.h"
#include "main.h"
#include "app.h"

#include <string.h>

/* 来自 u8g2 组件 */
extern I2C_HandleTypeDef hi2c1;

/* ------------------------------------------------------------------ */
/* 系统信息                                                            */
/* ------------------------------------------------------------------ */
int cmd_info(int argc, char *argv[])
{
    Shell *sh = shellGetCurrent();
    (void)argc;
    (void)argv;

    shellPrint(sh, "MCU    : STM32F103VCT6 (Cortex-M3)\r\n");
    shellPrint(sh, "SYSCLK : %lu Hz\r\n", (unsigned long)HAL_RCC_GetSysClockFreq());
    shellPrint(sh, "HCLK   : %lu Hz\r\n", (unsigned long)HAL_RCC_GetHCLKFreq());
    shellPrint(sh, "PCLK1  : %lu Hz\r\n", (unsigned long)HAL_RCC_GetPCLK1Freq());
    shellPrint(sh, "PCLK2  : %lu Hz\r\n", (unsigned long)HAL_RCC_GetPCLK2Freq());
    shellPrint(sh, "reset  : RCC_CSR = 0x%08lX\r\n", (unsigned long)RCC->CSR);
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) |
                 SHELL_CMD_DISABLE_RETURN, info, cmd_info, show system information);

int cmd_uptime(int argc, char *argv[])
{
    Shell *sh = shellGetCurrent();
    uint32_t ms = HAL_GetTick();
    (void)argc;
    (void)argv;

    shellPrint(sh, "uptime : %lu.%03lu s\r\n",
               (unsigned long)(ms / 1000U), (unsigned long)(ms % 1000U));
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) |
                 SHELL_CMD_DISABLE_RETURN, uptime, cmd_uptime, show running time);

/* ------------------------------------------------------------------ */
/* LED                                                                 */
/* ------------------------------------------------------------------ */
int cmd_led(int argc, char *argv[])
{
    Shell *sh = shellGetCurrent();

    if (argc < 2)
    {
        shellPrint(sh, "usage: led on|off|toggle|auto\r\n");
        return -1;
    }

    if (strcmp(argv[1], "auto") == 0)
    {
        app_led_auto();
        shellPrint(sh, "led: back to UI heartbeat\r\n");
        return 0;
    }

    /* 接管 LED，停止 UI 心跳，否则会被主循环翻转覆盖 */
    app_led_manual();

    if (strcmp(argv[1], "on") == 0)
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        shellPrint(sh, "led: on\r\n");
    }
    else if (strcmp(argv[1], "off") == 0)
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        shellPrint(sh, "led: off\r\n");
    }
    else if (strcmp(argv[1], "toggle") == 0)
    {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        shellPrint(sh, "led: toggled\r\n");
    }
    else
    {
        shellPrint(sh, "unknown option: %s\r\n", argv[1]);
        return -1;
    }
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) |
                 SHELL_CMD_DISABLE_RETURN, led, cmd_led, led on|off|toggle|auto);

/* ------------------------------------------------------------------ */
/* I2C 扫描                                                            */
/* ------------------------------------------------------------------ */
int cmd_i2c(int argc, char *argv[])
{
    Shell *sh = shellGetCurrent();
    uint8_t found = 0;
    (void)argc;
    (void)argv;

    shellPrint(sh, "scanning I2C1 (PB6/PB7)...\r\n");
    for (uint8_t addr = 0x08; addr < 0x78; addr++)
    {
        if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 2, 10) == HAL_OK)
        {
            shellPrint(sh, "  found device @ 0x%02X\r\n", addr);
            found++;
        }
    }
    shellPrint(sh, "done, %u device(s)\r\n", (unsigned)found);
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) |
                 SHELL_CMD_DISABLE_RETURN, i2c, cmd_i2c, scan the I2C1 bus);

/* ------------------------------------------------------------------ */
/* OLED 消息                                                           */
/* ------------------------------------------------------------------ */
int cmd_oled(int argc, char *argv[])
{
    Shell *sh = shellGetCurrent();
    char buf[40];

    if (argc < 2)
    {
        shellPrint(sh, "usage: oled <text> | oled clear\r\n");
        return -1;
    }

    if (strcmp(argv[1], "clear") == 0)
    {
        app_set_message("");
        shellPrint(sh, "oled: cleared\r\n");
        return 0;
    }

    buf[0] = '\0';
    for (int i = 1; i < argc; i++)
    {
        if (i > 1)
        {
            strncat(buf, " ", sizeof(buf) - strlen(buf) - 1);
        }
        strncat(buf, argv[i], sizeof(buf) - strlen(buf) - 1);
    }

    app_set_message(buf);
    shellPrint(sh, "oled: '%s'\r\n", buf);
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) |
                 SHELL_CMD_DISABLE_RETURN, oled, cmd_oled, oled <text> | oled clear);

/* ------------------------------------------------------------------ */
/* 其他                                                                */
/* ------------------------------------------------------------------ */
int cmd_echo(int argc, char *argv[])
{
    Shell *sh = shellGetCurrent();

    for (int i = 1; i < argc; i++)
    {
        shellPrint(sh, "%s%s", argv[i], (i + 1 < argc) ? " " : "\r\n");
    }
    if (argc < 2)
    {
        shellPrint(sh, "\r\n");
    }
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) |
                 SHELL_CMD_DISABLE_RETURN, echo, cmd_echo, echo <text>);

int cmd_reboot(int argc, char *argv[])
{
    Shell *sh = shellGetCurrent();
    (void)argc;
    (void)argv;

    shellPrint(sh, "rebooting...\r\n");
    HAL_Delay(20);
    NVIC_SystemReset();
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) |
                 SHELL_CMD_DISABLE_RETURN, reboot, cmd_reboot, reset the MCU);
