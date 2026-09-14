#include "pwm.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"

static uint8_t duty_actual = 0;

void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // Relojes para GPIOB y TIM12 (APB1)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);

    // PB14 -> TIM12_CH1
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_TIM12);

    // Clock APB1 Timer = 84MHz / (83 + 1) = 1MHz -> 1 us por tick
    // Periodo = 1000 us -> Frecuencia PWM = 1 kHz
    TIM_TimeBaseStructure.TIM_Prescaler     = 83;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period      = 999;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM12, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity    = TIM_OCPolarity_Low;
    TIM_OCInitStructure.TIM_Pulse       = 0;
    TIM_OC1Init(TIM12, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM12, TIM_OCPreload_Enable);
    TIM_Cmd(TIM12, ENABLE);

    duty_actual = 0;
}

void PWM_SetDutyCycle(uint8_t duty)
{
    if (duty > 100) duty = 100;
    duty_actual = duty;

    uint16_t pulse = (uint16_t)((uint32_t)duty * 1000 / 100);
    TIM_SetCompare1(TIM12, pulse);
}

uint8_t PWM_GetDutyCycle(void)
{
    return duty_actual;
}
