#include "u8g2_port.h"
#include "main.h"

I2C_HandleTypeDef hi2c1;
u8g2_t u8g2;

/*
 * u8g2 byte level callback driven by the STM32 hardware I2C1 peripheral.
 * u8g2 splits every transfer into at most 1 control byte + 24 data bytes,
 * so a 32 byte buffer is large enough.
 */
static uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[32];
    static uint8_t buf_idx;
    uint8_t *data;

    switch (msg)
    {
    case U8X8_MSG_BYTE_SEND:
        data = (uint8_t *)arg_ptr;
        while (arg_int > 0)
        {
            if (buf_idx < sizeof(buffer))
            {
                buffer[buf_idx++] = *data;
            }
            data++;
            arg_int--;
        }
        break;

    case U8X8_MSG_BYTE_INIT:
        /* I2C1 is already configured in u8g2_port_init() */
        break;

    case U8X8_MSG_BYTE_SET_DC:
        /* not used in I2C mode */
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        buf_idx = 0;
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
        if (HAL_I2C_Master_Transmit(&hi2c1, u8x8_GetI2CAddress(u8x8),
                                    buffer, buf_idx, 100) != HAL_OK)
        {
            return 0;
        }
        break;

    default:
        return 0;
    }
    return 1;
}

/*
 * GPIO / delay callback. Only the delay messages are used by the
 * hardware I2C setup, all GPIO handling is a no-op here.
 */
static uint8_t u8x8_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    (void)u8x8;
    (void)arg_ptr;

    switch (msg)
    {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        break;

    case U8X8_MSG_DELAY_MILLI:
        HAL_Delay(arg_int);
        break;

    case U8X8_MSG_DELAY_10MICRO:
    {
        uint32_t n = (SystemCoreClock / 1000000U) * 10U / 4U;
        while (n--)
        {
            __NOP();
        }
    }
    break;

    case U8X8_MSG_DELAY_100NANO:
        __NOP();
        break;

    case U8X8_MSG_DELAY_NANO:
        __NOP();
        break;

    default:
        break;
    }
    return 1;
}

void u8g2_port_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable I2C1 and its GPIO clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* PB6 / PB7 as open-drain alternate function */
    GPIO_InitStruct.Pin = U8G2_I2C_SCL_PIN | U8G2_I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(U8G2_I2C_GPIO_PORT, &GPIO_InitStruct);

    /* Configure I2C1 as 7-bit master */
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = U8G2_I2C_SPEED;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }

    /* Bind u8g2 to the SSD1315 (SSD1306 compatible) 128x64 panel */
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0,
                                           u8x8_byte_stm32_hw_i2c,
                                           u8x8_gpio_and_delay_stm32);
    u8g2_SetI2CAddress(&u8g2, U8G2_I2C_ADDR);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_ClearBuffer(&u8g2);
    u8g2_SendBuffer(&u8g2);
}
