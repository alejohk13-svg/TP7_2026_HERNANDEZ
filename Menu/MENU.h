#ifndef __MENU_H
#define __MENU_H

#include "stdint.h"
#include "../pwm/pwm.h"
#include "../ev1527/ev1527.h"
#include "../GLOBAL/global.h" // Corregido: carpeta GLOBAL

#define TOTAL_OPCIONES  5
#define LINEAS_DISPLAY  4

typedef enum {
    PANTALLA_PRINCIPAL,
    PANTALLA_TECLA,
    PANTALLA_BACKLIGHT,
    PANTALLA_PWM,
    PANTALLA_SEGUIDOR,
    PANTALLA_VOLTIMETRO,
    PANTALLA_DAC_OCUPADO
} modo_pantalla_t;

void MENU_Init(void);
void MENU_MostrarOpciones(void);
void MENU_Update(char tecla);

void PERIFERICOS_Init(void);
uint16_t Read_ADC_Value(uint8_t canal);

#endif /* __MENU_H */
