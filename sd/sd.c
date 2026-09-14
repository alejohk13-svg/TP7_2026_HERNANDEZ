#include "sd.h"

static FATFS FatFs;
static FIL Fil;
static uint8_t sdMontada = 0;

static FRESULT SD_Montar(void)
{
    FRESULT fr;
    uint8_t reintentos = 3;

    if (sdMontada != 0)
    {
        return FR_OK;
    }

    do
    {
        fr = f_mount(&FatFs, "", 1);

        if (fr == FR_OK)
        {
            sdMontada = 1;
            return FR_OK;
        }

        for (volatile uint32_t i = 0; i < 50000; i++);

    } while (--reintentos > 0);

    return fr;
}

FRESULT SD_Init_FatFS(void)
{
    return SD_Montar();
}

FRESULT SD_Log_ExisteArchivo(const char *filename)
{
    FRESULT fr;
    FILINFO fileInfo;
    char path[64];

    fr = SD_Montar();

    if (fr != FR_OK)
    {
        return fr;
    }

    snprintf(path, sizeof(path), "TD2/%s", filename);

    fr = f_stat(path, &fileInfo);

    if (fr == FR_NO_PATH)
    {
        return FR_NO_FILE;
    }

    return fr;
}

FRESULT SD_Log_CrearYEscribir(const char *filename, const char *data)
{
    FRESULT fr;
    UINT bytesWritten;
    char path[64];

    fr = SD_Montar();

    if (fr != FR_OK)
    {
        return fr;
    }

    fr = f_mkdir("TD2");

    if (fr != FR_OK && fr != FR_EXIST)
    {
        return fr;
    }

    snprintf(path, sizeof(path), "TD2/%s", filename);

    fr = f_open(&Fil, path, FA_WRITE | FA_CREATE_NEW);

    if (fr != FR_OK)
    {
        return fr;
    }

    if (strlen(data) > 0)
    {
        fr = f_write(&Fil, data, strlen(data), &bytesWritten);

        if (bytesWritten != strlen(data))
        {
            fr = FR_DISK_ERR;
        }
    }

    f_close(&Fil);

    return fr;
}

FRESULT SD_Log_AgregarDatos(const char *filename, const char *data)
{
    FRESULT fr;
    UINT bytesWritten;
    char path[64];

    fr = SD_Montar();

    if (fr != FR_OK)
    {
        return fr;
    }

    snprintf(path, sizeof(path), "TD2/%s", filename);

    /* Abre solamente un archivo existente. */
    fr = f_open(&Fil, path, FA_WRITE | FA_OPEN_EXISTING);

    if (fr != FR_OK)
    {
        return fr;
    }

    /* Posiciona la escritura al final. */
    fr = f_lseek(&Fil, f_size(&Fil));

    if (fr == FR_OK && strlen(data) > 0)
    {
        fr = f_write(&Fil, data, strlen(data), &bytesWritten);

        if (bytesWritten != strlen(data))
        {
            fr = FR_DISK_ERR;
        }
    }

    f_close(&Fil);

    return fr;
}

FRESULT SD_Log_BorrarArchivo(const char *filename)
{
    FRESULT fr;
    char path[64];

    fr = SD_Montar();

    if (fr != FR_OK)
    {
        return fr;
    }

    snprintf(path, sizeof(path), "TD2/%s", filename);

    fr = f_unlink(path);

    /* TD2 inexistente equivale a archivo inexistente. */
    if (fr == FR_NO_PATH)
    {
        return FR_NO_FILE;
    }

    return fr;
}
