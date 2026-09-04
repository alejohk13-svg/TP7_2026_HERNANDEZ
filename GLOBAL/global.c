#include "global.h"

// Instanciacion de la variable global de exclusion mutua
volatile ControlOrigen_t OrigenActual = CONTROL_LIBRE;

static volatile uint32_t msTicks = 0;
static uint32_t segundos = 0;

void sysTime_Init(void)
{
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        while (1);
    }
}

void SysTick_Handler(void)
{
    static uint32_t divisor_1s = 0;

    msTicks++;

    divisor_1s++;
    if (divisor_1s >= 1000)
    {
        divisor_1s = 0;
        segundos++;
    }
}

void delay_ms(uint32_t ms)
{
    uint32_t currentTicks = msTicks;
    while ((msTicks - currentTicks) < ms);
}

void delay_us(uint32_t us)
{
    volatile uint32_t i;
    for(i = 0; i < (us * 4); i++)
    {

    }
}

uint32_t getSystick(void)
{
    return msTicks;
}

uint32_t getSeconds(void)
{
    return segundos;
}
