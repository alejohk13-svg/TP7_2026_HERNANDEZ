#ifndef SD_H_
#define SD_H_

#include "stm32f4xx.h"
#include "../fatfs/ff.h"
#include <stdio.h>
#include <string.h>

FRESULT SD_Init_FatFS(void);
FRESULT SD_Log_ExisteArchivo(const char *filename);
FRESULT SD_Log_CrearYEscribir(const char *filename, const char *data);

#endif /* SD_H_ */
