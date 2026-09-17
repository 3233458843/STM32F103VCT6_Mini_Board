#include "shell_port.h"

/*
 * USART1 <-> CH340x USB-TTL
 *   PA9  = USART1_TX
 *   PA10 = USART1_RX
 *   115200 8N1
 */
UART_HandleTypeDef huart1;
Shell              shell;

static char shellBuffer[512];

#define RXBUF_SIZE 256

static volatile uint8_t  s_rxbuf[RXBUF_SIZE];
static volatile uint16_t s_rx_head;
static volatile uint16_t s_rx_tail;
static uint8_t           s_rx_byte;

/* ------------------------------------------------------------------ */
/* USART1 底层                                                         */
/* ------------------------------------------------------------------ */
static void usart1_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_10;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);

    HAL_NVIC_SetPriority(USART1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1);
}

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uint16_t next = (uint16_t)((s_rx_head + 1) % RXBUF_SIZE);
        if (next != s_rx_tail)
        {
            s_rxbuf[s_rx_head] = s_rx_byte;
            s_rx_head = next;
        }
        HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        __HAL_UART_CLEAR_OREFLAG(huart);
        HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1);
    }
}

/* retarget newlib printf/putchar to USART1 */
int __io_putchar(int ch)
{
    uint8_t c = (uint8_t)ch;
    HAL_UART_Transmit(&huart1, &c, 1, 1000);
    return ch;
}

/* ------------------------------------------------------------------ */
/* letter-shell 读写回调                                                */
/* ------------------------------------------------------------------ */
static short shell_write(char *data, unsigned short len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 1000);
    return (short)len;
}

static short shell_read(char *data, unsigned short len)
{
    (void)len;

    if (s_rx_tail == s_rx_head)
    {
        return 0;
    }

    data[0] = (char)s_rxbuf[s_rx_tail];
    s_rx_tail = (uint16_t)((s_rx_tail + 1) % RXBUF_SIZE);
    return 1;
}

unsigned int shell_port_get_tick(void)
{
    return HAL_GetTick();
}

/* ------------------------------------------------------------------ */
/* 对外接口                                                            */
/* ------------------------------------------------------------------ */
void shell_port_init(void)
{
    usart1_init();

    shell.write = shell_write;
    shell.read  = shell_read;
    shellInit(&shell, shellBuffer, sizeof(shellBuffer));
}

void shell_port_task(void)
{
    shellTask(&shell);
}
