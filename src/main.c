#include "stm32f4xx.h"
#include "global.h"
#include "../LCD/LCD.h"
#include "../Menu/MENU.h"
#include "../Teclado/TECLADO.h"
#include "../usart/usart.h"

int main(void)
{
    SystemInit();
    SystemCoreClockUpdate();

    sysTime_Init();
    LCD_init();
    teclado_init();
    MENU_Init();

    USARTx_Init(115200);

    while (1)
    {

        teclado_task();
        char tecla_actual = teclado_getc();
        MENU_Update(tecla_actual);
        USART_Task();
    }
}
