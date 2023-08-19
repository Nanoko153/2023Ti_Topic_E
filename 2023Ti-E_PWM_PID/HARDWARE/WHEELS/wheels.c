#include "wheels.h"

void TIM3_IRQHandler()
{
  if(TIM_GetITStatus(TIM3, TIM_IT_Update)==1) //当发生中断时状态寄存器(TIMx_SR)的bit0会被硬件置1
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //状态寄存器(TIMx_SR)的bit0置0
		//LED0 =~LED0;

		Xianfu_Pwm();   
		Velocity1=Position_PID_1(Position1,Target1); //舵机PID控制，可以根据目标位置进行速度调整，离目标位置越远速度越快
		Velocity2=Position_PID_2(Position2,Target2); //舵机PID控制，可以根据目标位置进行速度调整，离目标位置越远速度越快
		Xianfu_Velocity();
		Set_Pwm(Velocity1,Velocity2);    //赋值给PWM寄存器
	}
}

void Wheel_Value_Init(void)
{
	Position_KP = P_KP;
	Position_KI = P_KI;
	Position_KD = P_KD;
	
	speed = 0.6;
	speed_x = 1;
	speed_y = 1;
	
	Position1 = 750;
	Target1 = 750;
	
	Position2 = 750;
	Target2 = 750;
}


/**************************************************************************
函数功能：绝对值函数
入口参数：int
返回  值：unsigned int
**************************************************************************/
int myabs(int a)
{ 		   
	  int temp;
		if(a<0)  temp=-a;  
	  else temp=a;
	  return temp;
}

/**************************************************************************
函数功能：赋值给PWM寄存器,并且判断转向
入口参数：左轮PWM、右轮PWM
返回  值：无
**************************************************************************/
void Set_Pwm(float velocity1,float velocity2)
{	
	Position1+=velocity1 * speed * speed_x;		   //速度的积分，得到舵机的位置
	Position2+=velocity2 * speed * speed_y;		   //速度的积分，得到舵机的位置
	//赋值给STM32的寄存器
	TIM_SetCompare1(TIM2, Position1);
	TIM_SetCompare2(TIM2, Position2);
}
/**************************************************************************
函数功能：限制PWM赋值 
入口参数：无
返回  值：无
**************************************************************************/
void Xianfu_Pwm(void)
{	
	  //舵机1脉宽极限值，即限制舵机转角13.5°-256.5°
	  //舵机2脉宽极限值，即限制舵机转角9°   -171°
	int Amplitude_H=1200, Amplitude_L=350;       
    if(Target1<Amplitude_L)  Target1=Amplitude_L;	
	if(Target1>Amplitude_H)  Target1=Amplitude_H;	
	if(Target2<Amplitude_L)  Target2=Amplitude_L;	
	if(Target2>Amplitude_H)  Target2=Amplitude_H;	
}
/**************************************************************************
函数功能：限制速度 
入口参数：无
返回  值：无
**************************************************************************/
void Xianfu_Velocity(void)
{	
	int Amplitude_H=WHEEL_SPEED_MAX;
	int Amplitude_L=WHEEL_SPEED_MIN;  
	if(Velocity1<Amplitude_L)  Velocity1=Amplitude_L;	
	if(Velocity1>Amplitude_H)  Velocity1=Amplitude_H;	
	
	if(Velocity2<Amplitude_L)  Velocity2=Amplitude_L;	
	if(Velocity2>Amplitude_H)  Velocity2=Amplitude_H;	
}

/*************************************************************************
函数功能：位置式PID控制器1
入口参数：编码器测量位置信息，目标位置
返回  值：电机PWM增量
**************************************************************************/
float Position_PID_1(float Position,float Target)
{ 	                                          //增量输出
	 static float Bias,Pwm,Integral_bias,Last_Bias;
	 Bias=Target-Position;                                  //计算偏差
	 Integral_bias+=Bias;	                                 //求出偏差的积分
	 Pwm=Position_KP*Bias/100+Position_KI*Integral_bias/100+Position_KD*(Bias-Last_Bias)/100;       //位置式PID控制器
	 Last_Bias=Bias;                                       //保存上一次偏差 
	 return Pwm;  
}


/*************************************************************************
函数功能：位置式PID控制器2
入口参数：编码器测量位置信息，目标位置
返回  值：电机PWM增量
**************************************************************************/
float Position_PID_2(float Position,float Target)
{ 	                                          //增量输出
	 static float Bias,Pwm,Integral_bias,Last_Bias;
	 Bias=Target-Position;                                  //计算偏差
	 Integral_bias+=Bias;	                                 //求出偏差的积分
	 Pwm=Position_KP*Bias/100+Position_KI*Integral_bias/100+Position_KD*(Bias-Last_Bias)/100;       //位置式PID控制器
	 Last_Bias=Bias;                                       //保存上一次偏差 
	 return Pwm;  
}

//---------------原点复位---------------//
void Reset_Wheels(int center_X,int center_Y)
{
	while((myabs(Cx - center_X) > 1) || (myabs(Cy - center_Y)) > 1)
	{
		Target1 = Position1 + (Cx - center_X);
		Target2 = Position2 + (Cy - center_Y);
	}
	delay_ms(100);
	while((myabs(Cx - center_X) > 1) || (myabs(Cy - center_Y)) > 1)
	{
		Target1 = Position1 + (Cx - center_X);
		Target2 = Position2 + (Cy - center_Y);
	}
	delay_ms(100);
	while((myabs(Cx - center_X) > 1) || (myabs(Cy - center_Y)) > 1)
	{
		Target1 = Position1 + (Cx - center_X);
		Target2 = Position2 + (Cy - center_Y);
	}
	delay_ms(100);
	LED0 = 1;
}

void Asix_Control(int arrX,int arrY)
{
	/*
	while((myabs(Cx - arrX) >= 2) || (myabs(Cy - arrY)) >= 2)
	{
		LED0 = 0;
		Target1 = Position1 + (float)(Cx - arrX)*0.5;
		Target2 = Position2 + (float)(Cy - arrY)*0.5;
		
		printf("Target1:%.1f\r\n",Target1);
		printf("Target2:%.1f\r\n",Target2);
	}
	LED0 = 1;
	*/
	
	
	while(myabs( Cx - arrX) >= 2 || myabs( Cy - arrY) >= 2)
	{
		//printf("Moveing To P%d Cx:%d Cy:%d ----> Ax:%d Ay:%d T1:%.1f T2:%.1f\r\n",i+1,Cx,Cy,Screen_POS[0].x,Screen_POS[0].y,Target1,Target2);
		if( Cx - arrX > 0)
		{
			Target1 = Position1 + 2.5;
		}
		else if( Cx - arrX < 0)
		{
			Target1 = Position1 - 2.5;
		}
				
		//printf("Moveing To P%d Cx:%d Cy:%d ----> Ax:%d Ay:%d T1:%.1f T2:%.1f\r\n",i+1,Cx,Cy,Screen_POS[0].x,Screen_POS[0].y,Target1,Target2);
		if( Cy - arrY > 0)
		{
			Target2 = Position2 + 2.5;
		}
		else if( Cy - arrY < 0)
		{
			Target2 = Position2 - 2.5;
		}		
	}
	LED0 = 1;
	delay_ms(10);
}
