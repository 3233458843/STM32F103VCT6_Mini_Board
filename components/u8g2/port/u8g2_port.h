#pragma once

#include "u8g2.h"
#include "stm32f1xx_hal.h"

/* I2C1 pin mapping: PB6 = SCL, PB7 = SDA (I2C1 default pins) */
#define U8G2_I2C_SCL_PIN    GPIO_PIN_6
#define U8G2_I2C_SDA_PIN    GPIO_PIN_7
#define U8G2_I2C_GPIO_PORT  GPIOB
#define U8G2_I2C_SPEED      400000U

/* SSD1315 is SSD1306 compatible, 7-bit slave address 0x3C.
 * u8g2 expects the 8-bit (already shifted) form. */
#define U8G2_I2C_ADDR       (0x3C << 1)

extern I2C_HandleTypeDef hi2c1;
extern u8g2_t u8g2;

void u8g2_port_init(void);
