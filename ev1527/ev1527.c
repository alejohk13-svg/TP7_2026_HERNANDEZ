#include "ev1527.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"
#include "stm32f4xx_tim.h"

static volatile uint32_t codigo_recibido = 0;
static volatile uint8_t  trama_lista = 0;

static volatile uint32_t tiempo_alto = 0;
static volatile uint32_t tiempo_bajo = 0;
static volatile uint32_t ultimo_tiempo = 0;
static volatile uint32_t trama_tmp = 0;
static volatile uint8_t  bit_count = 0;

void EV1527_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    // Relojes para GPIOD, SYSCFG y TIM2
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // Configurar PD1 como entrada
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // Configurar TIM2 a 1 MHz (1 tick = 1 us)
    TIM_TimeBaseStructure.TIM_Prescaler     = 83; // 84MHz / 84 = 1MHz
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period        = 0xFFFFFFFF;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM2, ENABLE);

    // Conectar EXTI1 a PD1
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource1);

    // EXTI1 en ambos flancos (Rising y Falling)
    EXTI_InitStructure.EXTI_Line    = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // NVIC para EXTI1
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    codigo_recibido = 0;
    trama_lista = 0;
    bit_count = 0;
    trama_tmp = 0;
}

uint8_t EV1527_Disponible(void)
{
    return trama_lista;
}

uint32_t EV1527_GetCodigo(void)
{
    trama_lista = 0;
    return codigo_recibido;
}

void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        uint32_t ahora = TIM_GetCounter(TIM2);
        uint32_t duracion = ahora - ultimo_tiempo;
        ultimo_tiempo = ahora;

        if (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1) == RESET)
        {
            // Flanco descendente -> Mide duracion del pulso ALTO
            tiempo_alto = duracion;
        }
        else
        {
            // Flanco ascendente -> Mide duracion del pulso BAJO
            tiempo_bajo = duracion;

            // Detección del preámbulo / sincronismo: Alto CORTO seguido de Bajo LARGO
            if ((tiempo_alto >= MIN_CORTO && tiempo_alto <= MAX_CORTO) &&
                (tiempo_bajo >= MIN_SYNC  && tiempo_bajo <= MAX_SYNC))
            {
                bit_count = 0;
                trama_tmp = 0;
            }
            else if (bit_count < 24)
            {
                if (tiempo_alto >= MIN_CORTO && tiempo_alto <= MAX_CORTO)
                {
                    trama_tmp = (trama_tmp << 1);
                    bit_count++;
                }
                else if (tiempo_alto >= MIN_LARGO && tiempo_alto <= MAX_LARGO)
                {
                    trama_tmp = (trama_tmp << 1) | 1;
                    bit_count++;
                }
                else
                {
                    bit_count = 0;
                    trama_tmp = 0;
                }
                if (bit_count == 24)
                {
                    codigo_recibido = trama_tmp;
                    trama_lista = 1;
                    bit_count = 0;
                }
            }
        }

        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}
