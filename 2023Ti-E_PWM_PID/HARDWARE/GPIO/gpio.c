#include "gpio.h"

void LED_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC , ENABLE); 						 
	 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	LED0 = 1;
	delay_ms(150);
	LED0 = 0;
	delay_ms(150);
	LED0 = 1;
	delay_ms(150);
	LED0 = 0;
	delay_ms(150);
	LED0 = 1;
}

void Input_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	  
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE); 						 
		 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	LED0 = 1;
	delay_ms(200);
	LED0 = 0;
	delay_ms(250);
	LED0 = 1;
	delay_ms(300);
	LED0 = 0;
	delay_ms(350);
	LED0 = 1;
}

/*
void Wheels_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;            		//����ṹ��GPIO_InitStructure
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE); 	// ʹ��PB�˿�ʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;	  //PB7 PB8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     	//���죬��������������  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //IO���ٶ�
	GPIO_Init(GPIOA, &GPIO_InitStructure);          //GBIOB��ʼ��  

}
*/
