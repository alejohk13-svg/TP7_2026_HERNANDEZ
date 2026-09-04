#include "usart.h"
#include "../GLOBAL/global.h"
#include "../ev1527/ev1527.h"

static UART_MenuState_t estadoMenu = UART_MENU_MAIN;
static char rxBuffer[32];
static uint8_t rxIndex = 0;

// Variables filtro anti-rebote de RF
static uint32_t ultimo_tiempo_rf = 0;
static uint32_t ultimo_codigo_rf = 0;

void USARTx_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB1PeriphClockCmd(USARTx_CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(USARTx_GPIO_CLK, ENABLE);

    GPIO_PinAFConfig(GPIOD, USARTx_TX_SOURCE, USARTx_TX_AF);
    GPIO_PinAFConfig(GPIOD, USARTx_RX_SOURCE, USARTx_RX_AF);

    GPIO_InitStructure.GPIO_Pin   = USARTx_TX_PIN | USARTx_RX_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate            = baudrate;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USARTx, &USART_InitStructure);
    USART_Cmd(USARTx, ENABLE);
}

void USART_SendChar(char c)
{
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
    USART_SendData(USARTx, c);
}

void USART_SendString(const char *str)
{
    while (*str)
    {
        USART_SendChar(*str++);
    }
}

void USART_MostrarMenuPrincipal(void)
{
    USART_SendString("\r\n=== MENU PRINCIPAL UART ===\r\n");
    USART_SendString("1. Control de DAC\r\n");
    USART_SendString("2. Lectura RF\r\n");
    USART_SendString("3. Control Baudrate\r\n");
    USART_SendString("Seleccione una opcion (1-3): ");
}

void USART_Task(void)
{
    if (estadoMenu == UART_MENU_RF)
    {
        if (EV1527_Disponible())
        {
            uint32_t codigo = EV1527_GetCodigo();
            uint32_t tiempo_actual = getSystick();

            // Filtro anti-rebote: descarta si es el mismo código dentro de 350 ms
            if (codigo != ultimo_codigo_rf || (tiempo_actual - ultimo_tiempo_rf) > 350)
            {
                ultimo_codigo_rf = codigo;
                ultimo_tiempo_rf = tiempo_actual;

                char msg[80];

                switch (codigo)
                {
                    case EV1527_BOTON_A:
                        sprintf(msg, "[RF] ID Boton: 0x%06X | Boton A presionado!\r\n", (unsigned int)codigo);
                        USART_SendString(msg);
                        break;
                    case EV1527_BOTON_B:
                        sprintf(msg, "[RF] ID Boton: 0x%06X | Boton B presionado!\r\n", (unsigned int)codigo);
                        USART_SendString(msg);
                        break;
                    case EV1527_BOTON_C:
                        sprintf(msg, "[RF] ID Boton: 0x%06X | Boton C presionado!\r\n", (unsigned int)codigo);
                        USART_SendString(msg);
                        break;
                    case EV1527_BOTON_D:
                        sprintf(msg, "[RF] ID Boton: 0x%06X | Boton D presionado!\r\n", (unsigned int)codigo);
                        USART_SendString(msg);
                        break;
                    default:
                        sprintf(msg, "[RF] ID Boton: 0x%06X (Desconocido)\r\n", (unsigned int)codigo);
                        USART_SendString(msg);
                        break;
                }
            }
        }
    }

    if (USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) != RESET)
    {
        char c = (char)USART_ReceiveData(USARTx);

        if (c == '\r' || c == '\n')
        {
            USART_SendString("\r\n");

            if (rxIndex > 0)
            {
                rxBuffer[rxIndex] = '\0';

                switch (estadoMenu)
                {
                    case UART_MENU_MAIN:
                    {
                        int opcion = 0;
                        char basura[32];

                        if (sscanf(rxBuffer, "%d%s", &opcion, basura) == 1)
                        {
                            if (opcion == 1)
                            {
                                if (OrigenActual == CONTROL_LCD)
                                {
                                    USART_SendString("ERROR: DAC ocupado por LCD!\r\n");
                                    USART_MostrarMenuPrincipal();
                                }
                                else
                                {
                                    OrigenActual = CONTROL_UART;
                                    estadoMenu = UART_MENU_DAC;
                                    USART_SendString("Ingrese valor milivoltios (0 a 3300 mV): ");
                                }
                            }
                            else if (opcion == 2)
                            {
                                estadoMenu = UART_MENU_RF;
                                USART_SendString("\r\n--- LECTURA RF EN VIVO ---\r\n");
                                USART_SendString("Presione botones del mando RF. Presione ENTER vacio para salir...\r\n");
                            }
                            else if (opcion == 3)
                            {
                                estadoMenu = UART_MENU_CONFIG;
                                USART_SendString("Ingrese nuevo baudrate (ej. 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200): ");
                            }
                            else
                            {
                                USART_SendString("ERROR: Opcion invalida. Ingrese 1, 2 o 3.\r\n");
                                USART_MostrarMenuPrincipal();
                            }
                        }
                        else
                        {
                            USART_SendString("ERROR: Entrada no valida.\r\n");
                            USART_MostrarMenuPrincipal();
                        }
                        break;
                    }

                    case UART_MENU_DAC:
                    {
                        int mv = 0;
                        char basura[32];

                        if (sscanf(rxBuffer, "%d%s", &mv, basura) == 1)
                        {
                            if (mv >= 0 && mv <= 3300)
                            {
                                uint32_t cuentas = ((uint32_t)mv * 4095) / 3300;
                                DAC_SetChannel2Data(DAC_Align_12b_R, cuentas);

                                char bufferOut[64];
                                sprintf(bufferOut, "DAC actualizado a %d mV.\r\n", mv);
                                USART_SendString(bufferOut);

                                OrigenActual = CONTROL_LIBRE;
                                estadoMenu = UART_MENU_MAIN;
                                USART_MostrarMenuPrincipal();
                            }
                            else
                            {
                                USART_SendString("ERROR: Valor fuera de rango (0 a 3300 mV).\r\n");
                                USART_SendString("Ingrese valor milivoltios (0 a 3300 mV): ");
                            }
                        }
                        else
                        {
                            USART_SendString("ERROR: Debe ingresar un valor puramente numerico.\r\n");
                            USART_SendString("Ingrese valor milivoltios (0 a 3300 mV): ");
                        }
                        break;
                    }

                    case UART_MENU_RF:
                    {
                        USART_SendString("Presione ENTER vacio para salir del modo RF.\r\n");
                        break;
                    }

                    case UART_MENU_CONFIG:
                    {
                        uint32_t nuevoBaud = 0;
                        char basura[32];

                        if (sscanf(rxBuffer, "%lu%s", &nuevoBaud, basura) == 1)
                        {
                            if (nuevoBaud >= 1200 && nuevoBaud <= 2500000)
                            {
                                char bufferOut[64];
                                sprintf(bufferOut, "Reconfigurando UART a %lu baudios...\r\n", nuevoBaud);
                                USART_SendString(bufferOut);

                                while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);

                                USARTx_Init(nuevoBaud);

                                estadoMenu = UART_MENU_MAIN;
                                USART_MostrarMenuPrincipal();
                            }
                            else
                            {
                                USART_SendString("ERROR: Baudrate fuera de rango permitido.\r\n");
                                USART_SendString("Ingrese nuevo baudrate: ");
                            }
                        }
                        else
                        {
                            USART_SendString("ERROR: Debe ingresar un valor puramente numerico.\r\n");
                            USART_SendString("Ingrese nuevo baudrate: ");
                        }
                        break;
                    }

                    default:
                        estadoMenu = UART_MENU_MAIN;
                        USART_MostrarMenuPrincipal();
                        break;
                }

                rxIndex = 0;
            }
            else
            {
                if (estadoMenu == UART_MENU_DAC)
                {
                    OrigenActual = CONTROL_LIBRE;
                }
                estadoMenu = UART_MENU_MAIN;
                USART_MostrarMenuPrincipal();
            }
        }
        else
        {
            if (rxIndex < (sizeof(rxBuffer) - 1))
            {
                rxBuffer[rxIndex++] = c;
                USART_SendChar(c);
            }
        }
    }
}
