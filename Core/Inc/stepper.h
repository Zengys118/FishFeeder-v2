#ifndef __STEPPER_H
#define __STEPPER_H

#include "main.h"
#include <stdint.h>

void Stepper_Init(void);

void Stepper_Forward(uint32_t steps, uint16_t delay_ms);

void Stepper_Reverse(uint32_t steps, uint16_t delay_ms);

void Stepper_Release(void);

#endif
