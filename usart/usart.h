#ifndef USART_H_
#define USART_H_

#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../GLOBAL/global.h"
#include "../ev1527/ev1527.h"
#include "../sd/sd.h"

#define USARTx                   USART2
#define USARTx_CLK               RCC_APB1Periph_USART2
#define USARTx_GPIO_CLK          RCC_AHB1Periph_GPIOD

#define USARTx_TX_PIN            GPIO_Pin_5
#define USARTx_TX_SOURCE         GPIO_PinSource5
#define USARTx_TX_AF             GPIO_AF_USART2

#define USARTx_RX_PIN            GPIO_Pin_6
#define USARTx_RX_SOURCE         GPIO_PinSource6
#define USARTx_RX_AF             GPIO_AF_USART2

typedef enum {
    UART_MENU_MAIN = 0,
    UART_MENU_DAC,
    UART_MENU_RF,
    UART_MENU_CONFIG,
    UART_MENU_LOG_FILENAME,
    UART_MENU_LOG_DATA
} UART_MenuState_t;

void USARTx_Init(uint32_t baudrate);
void USART_SendChar(char c);
void USART_SendString(const char *str);
void USART_MostrarMenuPrincipal(void);
void USART_Task(void);

#endif /* USART_H_ */
