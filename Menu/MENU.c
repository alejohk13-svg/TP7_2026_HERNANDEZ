#include "MENU.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dac.h"
#include "stdio.h"
#include "../LCD/LCD.h"
#include "../Teclado/TECLADO.h"
#include "../GLOBAL/global.h"
#include "../ev1527/ev1527.h"

static modo_pantalla_t modo_actual = PANTALLA_PRINCIPAL;
static uint8_t opcion_seleccionada = 0;
static uint8_t ventana_inicio = 0;
static uint8_t backlight_encendido = 1;
static uint32_t localSystickContador = 0;

static uint32_t mv_hold = 0;

static const char *opciones_menu[] = {
    "1.Tecla Presionada ",
    "2.Backlight        ",
    "3.Control PWM      ",
    "4.Seguidor ADC-DAC ",
    "5.Voltimetro       "
};

void PERIFERICOS_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef  ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    DAC_InitTypeDef  DAC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    GPIO_StructInit(&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_CommonInitStructure.ADC_Mode             = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler        = ADC_Prescaler_Div4;
    ADC_CommonInitStructure.ADC_DMAAccessMode    = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution           = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode          = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode    = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge  = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign             = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion       = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    ADC_Cmd(ADC1, ENABLE);

    DAC_InitStructure.DAC_Trigger        = DAC_Trigger_None;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_OutputBuffer   = DAC_OutputBuffer_Enable;
    DAC_Init(DAC_Channel_2, &DAC_InitStructure);
    DAC_Cmd(DAC_Channel_2, ENABLE);

    PWM_Init();
    EV1527_Init();
}

uint16_t Read_ADC_Value(uint8_t canal)
{
    ADC_RegularChannelConfig(ADC1, canal, 1, ADC_SampleTime_3Cycles);
    ADC_SoftwareStartConv(ADC1);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    return ADC_GetConversionValue(ADC1);
}

void MENU_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_SetBits(GPIOD, GPIO_Pin_8);
    backlight_encendido = 1;

    PERIFERICOS_Init();

    LCD_clrscr();
    modo_actual = PANTALLA_PRINCIPAL;
    localSystickContador = 0;
    mv_hold = 0;
    MENU_MostrarOpciones();
}

void MENU_MostrarOpciones(void)
{
    uint8_t i;
    uint8_t indice_opcion;

    for (i = 0; i < LINEAS_DISPLAY; i++)
    {
        indice_opcion = ventana_inicio + i;
        LCD_gotoxy(0, i);

        if (indice_opcion == opcion_seleccionada)
        {
            LCD_putc('>');
        }
        else
        {
            LCD_putc(' ');
        }

        LCD_puts((char *)opciones_menu[indice_opcion]);
    }
}

void MENU_Update(char tecla)
{
    char buffer[20];
    uint16_t adc_raw = 0;
    uint32_t mv_pin_micro = 0;
    uint32_t mv_actual = 0;
    uint32_t mv_dac_teorico = 0;
    uint32_t cuentas_dac = 0;
    uint32_t suma_adc = 0;
    int i = 0;

    switch (modo_actual)
    {
        case PANTALLA_PRINCIPAL:
            if (tecla == 'A')
            {
                if (opcion_seleccionada > 0)
                {
                    opcion_seleccionada--;
                    if (opcion_seleccionada < ventana_inicio)
                    {
                        ventana_inicio = opcion_seleccionada;
                    }
                    LCD_clrscr();
                    MENU_MostrarOpciones();
                }
            }
            else if (tecla == 'B')
            {
                if (opcion_seleccionada < (TOTAL_OPCIONES - 1))
                {
                    opcion_seleccionada++;
                    if (opcion_seleccionada >= (ventana_inicio + LINEAS_DISPLAY))
                    {
                        ventana_inicio++;
                    }
                    LCD_clrscr();
                    MENU_MostrarOpciones();
                }
            }
            else if (tecla == 'C')
            {
                if (opcion_seleccionada == 0)
                {
                    modo_actual = PANTALLA_TECLA;
                    LCD_clrscr();
                    LCD_WriteString(0, 0, "Tecla: --");
                }
                else if (opcion_seleccionada == 1)
                {
                    modo_actual = PANTALLA_BACKLIGHT;
                    LCD_clrscr();
                    LCD_WriteString(0, 0, "Backlight: ");
                    if (backlight_encendido) { LCD_WriteString(11, 0, "ON "); }
                    else { LCD_WriteString(11, 0, "OFF"); }
                }
                else if (opcion_seleccionada == 2)
                {
                    modo_actual = PANTALLA_PWM;
                    LCD_clrscr();
                    sprintf(buffer, "PWM:  %3d%%", PWM_GetDutyCycle());
                    LCD_WriteString(0, 0, buffer);
                    LCD_WriteString(0, 1, "RF: ------");
                }
                else if (opcion_seleccionada == 3)
                {
                    if (OrigenActual != CONTROL_LIBRE)
                    {
                        modo_actual = PANTALLA_DAC_OCUPADO;
                        LCD_clrscr();
                        LCD_WriteString(0, 0, "DAC Ocupado!   ");
                        LCD_WriteString(0, 1, "Uso desde UART ");
                    }
                    else
                    {
                        OrigenActual = CONTROL_LCD;
                        modo_actual = PANTALLA_SEGUIDOR;
                        LCD_clrscr();
                        localSystickContador = getSystick();
                        LCD_WriteString(0, 0, "AD1:    0 mV");
                        LCD_WriteString(0, 1, "DAC:    0 mV");
                    }
                }
                else if (opcion_seleccionada == 4)
                {
                    modo_actual = PANTALLA_VOLTIMETRO;
                    LCD_clrscr();
                    localSystickContador = getSystick();
                    mv_hold = 0;
                    sprintf(buffer, "HOLD: %4lu mV ", mv_hold);
                    LCD_WriteString(0, 0, buffer);
                    LCD_WriteString(0, 1, "AD2:    0 mV");
                }
            }
            else if (tecla == '#')
            {
                backlight_encendido = 1;
                GPIO_SetBits(GPIOD, GPIO_Pin_8);
            }
            else if (tecla == '*')
            {
                backlight_encendido = 0;
                GPIO_ResetBits(GPIOD, GPIO_Pin_8);
            }
            break;

        case PANTALLA_DAC_OCUPADO:
            if (tecla == 'D')
            {
                modo_actual = PANTALLA_PRINCIPAL;
                LCD_clrscr();
                MENU_MostrarOpciones();
            }
            break;

        case PANTALLA_TECLA:
            if (tecla == 'D')
            {
                modo_actual = PANTALLA_PRINCIPAL;
                LCD_clrscr();
                MENU_MostrarOpciones();
            }
            else if (tecla != 0)
            {
                LCD_gotoxy(7, 0);
                LCD_putc(tecla);
                LCD_putc(' ');
            }
            break;

        case PANTALLA_BACKLIGHT:
            if (tecla == 'D')
            {
                modo_actual = PANTALLA_PRINCIPAL;
                LCD_clrscr();
                MENU_MostrarOpciones();
            }
            else if (tecla == '#')
            {
                backlight_encendido = 1;
                GPIO_SetBits(GPIOD, GPIO_Pin_8);
                LCD_WriteString(11, 0, "ON ");
            }
            else if (tecla == '*')
            {
                backlight_encendido = 0;
                GPIO_ResetBits(GPIOD, GPIO_Pin_8);
                LCD_WriteString(11, 0, "OFF");
            }
            break;

        case PANTALLA_PWM:
            if (tecla == 'D')
            {
                modo_actual = PANTALLA_PRINCIPAL;
                LCD_clrscr();
                MENU_MostrarOpciones();
            }
            else
            {
                uint8_t duty_actual = PWM_GetDutyCycle();
                uint8_t cambio = 0;
                static uint32_t ultimo_rf_tick = 0;

                if (EV1527_Disponible())
                {
                    uint32_t codigo = EV1527_GetCodigo();

                    sprintf(buffer, "RF: %06X", (unsigned int)codigo);
                    LCD_WriteString(0, 1, buffer);

                    if ((getSystick() - ultimo_rf_tick) >= 400)
                    {
                        ultimo_rf_tick = getSystick();

                        if (codigo == EV1527_BOTON_A)
                        {
                            if (duty_actual <= 95) duty_actual += 5;
                            else duty_actual = 100;
                            cambio = 1;
                        }
                        else if (codigo == EV1527_BOTON_B)
                        {
                            if (duty_actual >= 5) duty_actual -= 5;
                            else duty_actual = 0;
                            cambio = 1;
                        }
                    }
                }

                if (cambio)
                {
                    PWM_SetDutyCycle(duty_actual);
                    sprintf(buffer, "PWM:  %3d%%", duty_actual);
                    LCD_WriteString(0, 0, buffer);
                }
            }
            break;

        case PANTALLA_SEGUIDOR:
            if (tecla == 'D')
            {
                OrigenActual = CONTROL_LIBRE;
                modo_actual = PANTALLA_PRINCIPAL;
                LCD_clrscr();
                MENU_MostrarOpciones();
            }
            else
            {
                if ((getSystick() - localSystickContador) >= 650)
                {
                    localSystickContador = getSystick();

                    suma_adc = 0;
                    for (i = 0; i < 10; i++)
                    {
                        suma_adc += Read_ADC_Value(ADC_Channel_8);
                    }
                    adc_raw = suma_adc / 10;

                    if (adc_raw > 4095) adc_raw = 4095;

                    mv_pin_micro = ((uint32_t)adc_raw * 3300) / 4095;
                    mv_actual = (mv_pin_micro * (2200 + 4700)) / 4700;

                    if (mv_actual <= 3300)
                    {
                        mv_dac_teorico = mv_actual;
                        cuentas_dac = (mv_actual * 4095) / 3300;
                        if (cuentas_dac > 4095) cuentas_dac = 4095;
                        DAC_SetChannel2Data(DAC_Align_12b_R, cuentas_dac);
                    }
                    else
                    {
                        mv_dac_teorico = 3300;
                        DAC_SetChannel2Data(DAC_Align_12b_R, 4095);
                    }

                    sprintf(buffer, "AD1: %4lu mV ", mv_actual);
                    LCD_WriteString(0, 0, buffer);
                    sprintf(buffer, "DAC: %4lu mV ", mv_dac_teorico);
                    LCD_WriteString(0, 1, buffer);
                }
            }
            break;

        case PANTALLA_VOLTIMETRO:
            if (tecla == 'D')
            {
                modo_actual = PANTALLA_PRINCIPAL;
                LCD_clrscr();
                MENU_MostrarOpciones();
            }
            else
            {
                if ((getSystick() - localSystickContador) >= 650)
                {
                    localSystickContador = getSystick();

                    suma_adc = 0;
                    for (i = 0; i < 10; i++)
                    {
                        suma_adc += Read_ADC_Value(ADC_Channel_9);
                    }
                    adc_raw = suma_adc / 10;

                    if (adc_raw > 4095) adc_raw = 4095;

                    mv_pin_micro = ((uint32_t)adc_raw * 3300) / 4095;
                    mv_actual = (mv_pin_micro * (2200 + 4700)) / 4700;

                    if (tecla == '0')
                    {
                        mv_hold = mv_actual;
                        sprintf(buffer, "HOLD: %4lu mV ", mv_hold);
                        LCD_WriteString(0, 0, buffer);
                    }

                    sprintf(buffer, "AD2:  %4lu mV ", mv_actual);
                    LCD_WriteString(0, 1, buffer);
                }
                else if (tecla == '0')
                {
                    mv_hold = mv_actual;
                    sprintf(buffer, "HOLD: %4lu mV ", mv_hold);
                    LCD_WriteString(0, 0, buffer);
                }
            }
            break;

        default:
            modo_actual = PANTALLA_PRINCIPAL;
            break;
    }
}
