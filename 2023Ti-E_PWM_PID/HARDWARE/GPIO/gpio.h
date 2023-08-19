#ifndef __GPIO_H
#define	__GPIO_H

#include "stm32f10x.h"
#include "delay.h"

#define LED0 PCout(13)

#define INPUT0 PAin(4)
#define INPUT1 PAin(5)
#define INPUT2 PAin(6)
#define INPUT3 PAin(7)

void LED_GPIO_Init(void);
void Input_GPIO_Init(void);
//void Wheels_GPIO_Init(void);
void Input_GPIO_Init(void);

#endif

