#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "stm32f4xx.h"

// Tipos para exclusion mutua
typedef enum {
    CONTROL_LIBRE = 0,
    CONTROL_LCD,
    CONTROL_UART
} ControlOrigen_t;

// Alias para compatibilidad de tipos
typedef ControlOrigen_t OrigenControl_t;

// Declaracion de la variable global de exclusion
extern volatile ControlOrigen_t OrigenActual;

void sysTime_Init(void);
void SysTick_Handler(void);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
uint32_t getSystick(void);
uint32_t getSeconds(void);

#endif /* GLOBAL_H_ */
