#include "teclado.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

extern uint32_t getSystick(void);

typedef struct
{
    GPIO_TypeDef* puerto;
    uint16_t pin;
} pin_t;

pin_t filas[4] =
{
    {GPIOE, GPIO_Pin_8},
    {GPIOE, GPIO_Pin_9},
    {GPIOE, GPIO_Pin_10},
    {GPIOE, GPIO_Pin_11}
};

pin_t columns[4] =
{
    {GPIOC, GPIO_Pin_0},
    {GPIOC, GPIO_Pin_3},
    {GPIOC, GPIO_Pin_2},
    {GPIOA, GPIO_Pin_0}
};

char mapa[4][4] =
{
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

static char tecla = 0;

typedef enum
{
    ESTADO_INICIAL,
    ESTADO_DETECCION,
    ESTADO_CONFIRMACION,
    ESTADO_ESPERA_LIBERACION,
    ESTADO_DEBOUNCE_LIBERACION
} estado_t;

static estado_t estado = ESTADO_INICIAL;
static char tecla_detectada = 0;
static int contador = 0;

#define DEBOUNCE_DELAY 50

void teclado_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStruct.GPIO_Pin = filas[0].pin | filas[1].pin | filas[2].pin | filas[3].pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = columns[0].pin | columns[1].pin | columns[2].pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = columns[3].pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    tecla = 0;
    estado = ESTADO_INICIAL;
}

void filas_high(void)
{
    for (int i = 0; i < 4; i++)
    {
        GPIO_SetBits(filas[i].puerto, filas[i].pin);
    }
}

void activar_fila(int fila)
{
    filas_high();
    GPIO_ResetBits(filas[fila].puerto, filas[fila].pin);
}

int leer_columna(void)
{
    for (int i = 0; i < 4; i++)
    {
        if (GPIO_ReadInputDataBit(columns[i].puerto, columns[i].pin) == 0)
        {
            return i;
        }
    }
    return -1;
}

static char escanear_teclado(void)
{
    int col;
    for (int fila = 0; fila < 4; fila++)
    {
        activar_fila(fila);
        col = leer_columna();
        if (col != -1)
        {
            return mapa[fila][col];
        }
    }
    return 0;
}

void teclado_update(void)
{
    char t = escanear_teclado();

    switch(estado)
    {
        case ESTADO_INICIAL:
            tecla = 0;
            if (t != 0)
            {
                tecla_detectada = t;
                contador = 0;
                estado = ESTADO_DETECCION;
            }
            break;

        case ESTADO_DETECCION:
            contador++;
            if (contador >= DEBOUNCE_DELAY)
            {
                estado = ESTADO_CONFIRMACION;
            }
            break;

        case ESTADO_CONFIRMACION:
            if (escanear_teclado() == tecla_detectada)
            {
                tecla = tecla_detectada;
                estado = ESTADO_ESPERA_LIBERACION;
            }
            else
            {
                estado = ESTADO_INICIAL;
            }
            break;

        case ESTADO_ESPERA_LIBERACION:
            if (escanear_teclado() == 0)
            {
                contador = 0;
                estado = ESTADO_DEBOUNCE_LIBERACION;
            }
            break;

        case ESTADO_DEBOUNCE_LIBERACION:
            if (escanear_teclado() == 0)
            {
                contador++;
                if (contador >= DEBOUNCE_DELAY)
                {
                    estado = ESTADO_INICIAL;
                }
            }
            else
            {
                contador = 0;
                estado = ESTADO_ESPERA_LIBERACION;
            }
            break;

        default:
            estado = ESTADO_INICIAL;
            break;
    }
}

char teclado_getKey(void)
{
    char aux = tecla;
    tecla = 0;
    return aux;
}

int teclado_presionado(void)
{
    return (escanear_teclado() != 0);
}

static uint32_t last_tick_teclado_interno = 0;

void teclado_task(void)
{
    if ((getSystick() - last_tick_teclado_interno) >= 1)
    {
        last_tick_teclado_interno = getSystick();
        teclado_update();
    }
}

char teclado_getc(void)
{
    return teclado_getKey();
}
