#include "wheels.h"

void TIM3_IRQHandler()
{
  if(TIM_GetITStatus(TIM3, TIM_IT_Update)==1) //�������ж�ʱ״̬�Ĵ���(TIMx_SR)��bit0�ᱻӲ����1
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //״̬�Ĵ���(TIMx_SR)��bit0��0
		//LED0 =~LED0;

		Xianfu_Pwm();   
		Velocity1=Position_PID_1(Position1,Target1); //���PID���ƣ����Ը���Ŀ��λ�ý����ٶȵ�������Ŀ��λ��ԽԶ�ٶ�Խ��
		Velocity2=Position_PID_2(Position2,Target2); //���PID���ƣ����Ը���Ŀ��λ�ý����ٶȵ�������Ŀ��λ��ԽԶ�ٶ�Խ��
		Xianfu_Velocity();
		Set_Pwm(Velocity1,Velocity2);    //��ֵ��PWM�Ĵ���
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
�������ܣ�����ֵ����
��ڲ�����int
����  ֵ��unsigned int
**************************************************************************/
int myabs(int a)
{ 		   
	  int temp;
		if(a<0)  temp=-a;  
	  else temp=a;
	  return temp;
}

/**************************************************************************
�������ܣ���ֵ��PWM�Ĵ���,�����ж�ת��
��ڲ���������PWM������PWM
����  ֵ����
**************************************************************************/
void Set_Pwm(float velocity1,float velocity2)
{	
	Position1+=velocity1 * speed * speed_x;		   //�ٶȵĻ��֣��õ������λ��
	Position2+=velocity2 * speed * speed_y;		   //�ٶȵĻ��֣��õ������λ��
	//��ֵ��STM32�ļĴ���
	TIM_SetCompare1(TIM2, Position1);
	TIM_SetCompare2(TIM2, Position2);
}
/**************************************************************************
�������ܣ�����PWM��ֵ 
��ڲ�������
����  ֵ����
**************************************************************************/
void Xianfu_Pwm(void)
{	
	  //���1������ֵ�������ƶ��ת��13.5��-256.5��
	  //���2������ֵ�������ƶ��ת��9��   -171��
	int Amplitude_H=1200, Amplitude_L=350;       
    if(Target1<Amplitude_L)  Target1=Amplitude_L;	
	if(Target1>Amplitude_H)  Target1=Amplitude_H;	
	if(Target2<Amplitude_L)  Target2=Amplitude_L;	
	if(Target2>Amplitude_H)  Target2=Amplitude_H;	
}
/**************************************************************************
�������ܣ������ٶ� 
��ڲ�������
����  ֵ����
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
�������ܣ�λ��ʽPID������1
��ڲ���������������λ����Ϣ��Ŀ��λ��
����  ֵ�����PWM����
**************************************************************************/
float Position_PID_1(float Position,float Target)
{ 	                                          //�������
	 static float Bias,Pwm,Integral_bias,Last_Bias;
	 Bias=Target-Position;                                  //����ƫ��
	 Integral_bias+=Bias;	                                 //���ƫ��Ļ���
	 Pwm=Position_KP*Bias/100+Position_KI*Integral_bias/100+Position_KD*(Bias-Last_Bias)/100;       //λ��ʽPID������
	 Last_Bias=Bias;                                       //������һ��ƫ�� 
	 return Pwm;  
}


/*************************************************************************
�������ܣ�λ��ʽPID������2
��ڲ���������������λ����Ϣ��Ŀ��λ��
����  ֵ�����PWM����
**************************************************************************/
float Position_PID_2(float Position,float Target)
{ 	                                          //�������
	 static float Bias,Pwm,Integral_bias,Last_Bias;
	 Bias=Target-Position;                                  //����ƫ��
	 Integral_bias+=Bias;	                                 //���ƫ��Ļ���
	 Pwm=Position_KP*Bias/100+Position_KI*Integral_bias/100+Position_KD*(Bias-Last_Bias)/100;       //λ��ʽPID������
	 Last_Bias=Bias;                                       //������һ��ƫ�� 
	 return Pwm;  
}

//---------------ԭ�㸴λ---------------//
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
