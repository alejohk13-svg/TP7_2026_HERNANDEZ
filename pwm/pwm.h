#ifndef __PWM_H
#define __PWM_H

#include "stdint.h"

void PWM_Init(void);
void PWM_SetDutyCycle(uint8_t duty); // Recibe duty de 0 a 100%
uint8_t PWM_GetDutyCycle(void);

#endif /* __PWM_H */
