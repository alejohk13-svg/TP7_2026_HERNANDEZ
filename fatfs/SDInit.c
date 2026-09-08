#include "SDInit.h"
#include "stm32f4xx.h"
#include "misc.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_sdio_sd.h"


static void delay_ms(uint32_t ms)
{
    volatile uint32_t n;
    while (ms--) {
        for (n = 0; n < 20000; n++) {
        }
    }
}

void SD_Start(void)
{
    GPIO_InitTypeDef gpio;
    NVIC_InitTypeDef nvic;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, ENABLE);
    RCC_AHB1PeriphClockCmd(SD_SDIO_DMA_CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(SD_ONOFF_CLOCK, ENABLE);

    nvic.NVIC_IRQChannel = DMA2_Stream3_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    nvic.NVIC_IRQChannel = SDIO_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&nvic);

    delay_ms(50);
}

void SDIO_IRQHandler(void)
{
    SD_ProcessIRQSrc();
}

void DMA2_Stream3_IRQHandler(void)
{
    SD_ProcessDMAIRQ();
}
