#ifndef __EV1527_H
#define __EV1527_H

#include "stdint.h"

// Definiciones de Botones del Mando RF
#define EV1527_BOTON_A    0x7D3101
#define EV1527_BOTON_B    0x7D3102
#define EV1527_BOTON_C    0x7D3104
#define EV1527_BOTON_D    0x7D3108

// Pulso corto medido en osciloscopio: 368uS
#define MIN_CORTO         250
#define MAX_CORTO         450

// Pulso largo medido en osciloscopio: 1080uS
#define MIN_LARGO         950
#define MAX_LARGO         1200

// Sincronismo
#define MIN_SYNC          7000
#define MAX_SYNC          15000

// Funciones públicas
void EV1527_Init(void);
uint8_t EV1527_Disponible(void);
uint32_t EV1527_GetCodigo(void);

#endif /* __EV1527_H */
