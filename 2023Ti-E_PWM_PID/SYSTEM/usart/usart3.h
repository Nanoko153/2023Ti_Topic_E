#ifndef __USART3_H
#define __USART3_H

#include "sys.h"

extern int  Cx,Cy,Cw,Ch;
extern uint8_t Urxbuf[9]; //����3������������
extern int Usart_Compelet; //���ڽ�����һ�����ݱ�־

//CAM������������
//�����ĸ���������
extern int rectangle_Pos_1_X;
extern int rectangle_Pos_1_Y;
extern int rectangle_Pos_2_X;
extern int rectangle_Pos_2_Y;
extern int rectangle_Pos_3_X;
extern int rectangle_Pos_3_Y;
extern int rectangle_Pos_4_X;
extern int rectangle_Pos_4_Y;

extern int next_Pos_X;
extern int next_POS_Y;

void usart3_send(u8 data);
void My_USART3_Init(u32 bound)  ; 
void USART3_IRQHandler(void);

void usart3_sendAngleBlock(char order);
void CAM_Data_handle(void);
#endif
