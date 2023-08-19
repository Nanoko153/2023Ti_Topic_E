#ifndef __WHEELS_H
#define	__WHEELS_H

#include "delay.h"
#include "gpio.h"

#define W_X_Rotation_Speed 1			//X轴转向速度
#define W_X_Rotation_Wait_Ms 20			//X轴转向等待延时

#define W_Y_Rotation_Speed 1			//Y轴转向速度
#define W_Y_Rotation_Wait_Ms 20			//Y轴转向速度

#define Center_X 80
#define Center_Y 60

#define	P_KP 6;
#define P_KI 0;
#define P_KD 3;  //位置控制PID参数

#define WHEEL_SPEED_MAX 2;
#define WHEEL_SPEED_MIN -2;

extern int  Cx,Cy,Cw,Ch;	//当前x，y值
extern float Position_KP,Position_KI,Position_KD;  //位置控制PID参数

extern float speed;
extern float speed_x;
extern float speed_y;
extern float Velocity1,Velocity2;     //电机PWM变量
extern float Position1,Position2;    
extern float Target1,Target2;     //电机目标值
extern int w_X_PWM;
extern int w_Y_PWM;

extern int reset_Key;

void TIM3_Int_Init(u16 arr, u16 psc);
void Wheel_Value_Init(void);
void Set_Pwm(float velocity1,float velocity2);
void Xianfu_Pwm(void);
void Xianfu_Velocity(void);
float Position_PID_1(float Position,float Target);
float Position_PID_2(float Position,float Target);


void Reset_Wheels(int center_X,int center_Y);
void Asix_Control(int arrX,int arrY);
int myabs(int a);

#endif
