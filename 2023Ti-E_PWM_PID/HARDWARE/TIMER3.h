#ifndef __TIMER3_H
#define	__TIMER3_H

#include "sys.h"
#include "gpio.h"
#include "stm32f10x_tim.h"
#include "pid.h"
#include "usart.h"
#include "wheels.h"

void TIM3_Int_Init(u16 arr, u16 psc);

#endif
