//----------SYSTEM----------//
#include "stm32f10x.h"
#include <stdio.h>
#include <stdlib.h>
#include "delay.h"
#include "usart.h"
#include "usart3.h"
//----------GPIO----------//
#include "gpio.h"
//----------WEELS----------//
#include "TIMER.h"
#include "TIMER3.h"
#include "wheels.h"

//�궨��
#define MAX_ARR_LENG 20		//����1���������ݻ���
#define PTP_COUNT 5
#define SCREEN_POS_COUNT 9
struct POS_PWM
{
	int x;
	int y;
	float pwm_x;
	float pwm_y;
};

void Arr_Clear(char *arr,int len);
void Get_Message(void);
void Message_Handler(void);
void Input_Handler(void);

//---------------------------------------------------------------------------------------------//��������
//��һ��
void Get_Screen_POS(void);			//�����Ļ��Χ��

//�ڶ���		//����Ļ�ƶ�
float Pos_To_Screen_PWM_X(int pos);
float Pos_To_Screen_PWM_Y(int pos);
void Vertex_Init(void);
void Topic_2_Draw_screen(void);
void Topic_2_Draw_Screen_By_POS(void);

int vertet_count = 0;
struct POS_PWM vertex[5]; 
struct POS_PWM Screen_POS[SCREEN_POS_COUNT];

 

//������
void Get_PTP_POS(int start_X, int start_Y, int end_X, int end_Y);
void Topic_3_Draw_Rectangle(void);
struct POS_PWM ptp[PTP_COUNT];
int rectangle_Pos_1_X;//CAM������������
int rectangle_Pos_1_Y;//�����ĸ���������
int rectangle_Pos_2_X;
int rectangle_Pos_2_Y;
int rectangle_Pos_3_X;
int rectangle_Pos_3_Y;
int rectangle_Pos_4_X;
int rectangle_Pos_4_Y;


//����1
u8 str[20];					//�ַ�������
char arr[MAX_ARR_LENG];					//��Ϣ���黺��

//����3
uint8_t Urxbuf[9]; //����3������������
int Usart_Compelet; //���ڽ�����һ�����ݱ�־

//PWMֱ�أ������ã�
int w_X_PWM;
int w_Y_PWM;

//PID����
float speed;									//���ٶ�
float speed_x;									//x����ٶ�
float speed_y;									//y�����ٶ�
float Position_KP,Position_KI,Position_KD;  	//λ�ÿ���PID����
float Velocity1,Velocity2;    				 	//PWM����
float Position1,Position2;   					//�����ǰPWM
float Target1,Target2;							//���Ŀ��PWM

float min_X_PWM = 500;		//�ڶ��ⷶΧ
float max_X_PWM = 1000;
float min_Y_PWM = 500;
float max_Y_PWM = 1000;

int  Cx,Cy,Cw,Ch;	//��ǰx��yֵ

int next_Pos_X;
int next_POS_Y;

int mode = 99;		//ģʽ��־λ
int reset_Key;
int can_Get_Pos;
int main(void)
{
	int i;
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);
	My_USART3_Init(115200);
	
	LED_GPIO_Init();
	Input_GPIO_Init();
	
	Wheel_Value_Init();
	
	TIM3_Int_Init(100-1,7200-1);
	TIM2_PWM_Init(9999, 143); //TIM2_Int_Init(u16 arr, u16 psc)����ʼ����ʱ��TIM2
	                        //��ʱʱ��=��arr+1)(psc+1)/Tclk��TclkΪ�ڲ�ͨ�ö�ʱ��ʱ�ӣ�������Ĭ������Ϊ72MHZ
	
	Target1 = 750; Target2 = 750;
	
	usart3_sendAngleBlock(0X03);
	
	delay_ms(1000);
	
	usart3_sendAngleBlock(0X03);
	delay_ms(1000);
	
	usart3_sendAngleBlock(0X03);
	delay_ms(1000);
	
	Get_Screen_POS();
	
	while(1)
	{
		Get_Message();
		Message_Handler();
		Input_Handler();
		
		printf("%d\t%d\r\n",Cx,Cy);
		delay_ms(100);
		
		switch(mode)
		{
			case 0:
				Reset_Wheels(Screen_POS[SCREEN_POS_COUNT - 1].x,Screen_POS[SCREEN_POS_COUNT - 1].y);
				mode = 99;
			break;
			case 1:
				//Topic_2_Draw_screen();
				Topic_2_Draw_Screen_By_POS();
				Reset_Wheels(Screen_POS[SCREEN_POS_COUNT - 1].x,Screen_POS[SCREEN_POS_COUNT - 1].y);
				mode = 99;
			break;
			case 2:
				Topic_3_Draw_Rectangle();
				Reset_Wheels(Screen_POS[SCREEN_POS_COUNT - 1].x,Screen_POS[SCREEN_POS_COUNT - 1].y);
				mode = 99;
			break;
		}
	}
}
//��ʼ����ȡ�����
void Get_Screen_POS(void)
{
	int i,j;
	//LASER = 1;		//�򿪼���
	
	while(Cx == 0 && Cy == 0)
	{
		usart3_sendAngleBlock(0X03);		//���ʹ���������
		delay_ms(500);						//����ĵȴ�ʱ��
	}
	//Reset_Wheels();							//���û��������ĵ�
	//delay_ms(1000);							//�ȴ�2s����ص��������ĵ�
	//delay_ms(1000);
	//Screen_POS[0].x = Cx;	Screen_POS[0].y = Cy;	//�����ĵ㷵�ظ�Screen_POS�����е�0��λ
	//LASER = 0; 								//�رռ���
	LED0 = 0;
	delay_ms(500);
	LED0 = 1;
	delay_ms(500);
	LED0 = 0;
	delay_ms(500);
	LED0 = 1;
	delay_ms(500);
	LED0 = 0;
	delay_ms(500);
	LED0 = 1;
	for(i = 0; i < SCREEN_POS_COUNT ;i++)	//��ʼ������Ļ�޶�����
	{
		while(!can_Get_Pos)
		{
			if(INPUT0 == 1)
			{
				delay_ms(10);
				if(INPUT0 == 1)
				{
					while(INPUT0 == 1);			//�ȴ�����
					can_Get_Pos = 1;			//��д��������־λ
					LED0 = 0;
				}
			}
		}
		Screen_POS[i].x = Cx; Screen_POS[i].y = Cy; //��õڶ�������
		can_Get_Pos = 0;							//�ر������ȡ�ȴ���һ�ζ�ȡ��־
		printf("P%d:x_%d\ty_%d\r\n",i,Screen_POS[i].x,Screen_POS[i].y);
		for(j = 0;j < i; j++)	//��˸��Ӧ����
		{
			LED0 = 0;
			delay_ms(100);
			LED0 = 1;
			delay_ms(100);
		}
	}
	
	delay_ms(500);
	LED0 = 0;
	delay_ms(1000);
	LED0 = 1;
	
	for(i = 0;i<SCREEN_POS_COUNT;i++)
	{
		printf("P%d:x_%d\ty_%d\r\n",i,Screen_POS[i].x,Screen_POS[i].y);
	}
}
//��һ�� ��λ


//�ڶ���
float Pos_To_Screen_PWM_X(int pos)
{
	float pwm;
	float temp_pwm;
	float temp_pos;
	float percent;
	if(pos < vertex[0].x)
	{
		temp_pwm = (( vertex[1].pwm_x - vertex[0].pwm_x)+(vertex[4].pwm_x - vertex[0].pwm_x))/2; //�����Ļ���ߵ�ƽ��PWM��ֵ
		temp_pos = (float)(( vertex[0].x - vertex[1].x)+(vertex[0].x - vertex[4].x))/2;
		percent = (float)(vertex[0].x - pos)/temp_pos;
		pwm = vertex[0].pwm_x + temp_pwm - temp_pwm * percent;
		printf("temp_pwm:%.1f\t",temp_pwm);
		printf("temp_pos:%.1f\t",temp_pos);
		printf("percent:%.1f\t",percent);
		printf("pwm:%.1f\t",pwm);
	}
	else if(pos > vertex[0].x)
	{
		temp_pwm = (( vertex[0].pwm_x - vertex[2].pwm_x)+(vertex[0].pwm_x - vertex[3].pwm_x))/2; //�����Ļ�Ұ�ߵ�ƽ��PWM��ֵ
		temp_pos = (float)(( vertex[2].x - vertex[0].x)+(vertex[3].x - vertex[0].x))/2;
		percent = (float)(pos - vertex[0].x)/temp_pos;
		pwm = vertex[0].pwm_x - temp_pwm - temp_pwm * percent;
		printf("temp_pwm:%.1f\t",temp_pwm);
		printf("temp_pos:%.1f\t",temp_pos);
		printf("percent:%.1f\t",percent);
		printf("pwm:%.1f\t",pwm);
	}
	else
		pwm = vertex[0].pwm_x;
	return pwm;
}

float Pos_To_Screen_PWM_Y(int pos)
{
	float pwm;
	float temp_pwm;
	float temp_pos;
	float percent;
	if(pos < vertex[0].y)
	{
		temp_pwm = (( vertex[0].pwm_y - vertex[3].pwm_y)+(vertex[0].pwm_y - vertex[4].pwm_y))/2; //�����Ļ���ߵ�ƽ��PWM��ֵ
		temp_pos = (float)(( vertex[0].y - vertex[3].y)+(vertex[0].y - vertex[4].y))/2;
		percent = (float)(vertex[0].y - pos)/temp_pos;
		pwm = vertex[0].pwm_y - temp_pwm - temp_pwm * percent;
		printf("temp_pwm:%.1f\t",temp_pwm);
		printf("temp_pos:%.1f\t",temp_pos);
		printf("percent:%.1f\t",percent);
		printf("pwm:%.1f\t",pwm);
	}
	else if(pos > vertex[0].y)
	{
		temp_pwm = (( vertex[1].pwm_y - vertex[0].pwm_y)+(vertex[2].pwm_y - vertex[0].pwm_y))/2; //�����Ļ�Ұ�ߵ�ƽ��PWM��ֵ
		temp_pos = (float)(( vertex[2].y - vertex[0].y)+(vertex[3].y - vertex[0].y))/2;
		percent = (float)(pos - vertex[0].y)/temp_pos;
		pwm = vertex[0].pwm_y + temp_pwm - temp_pwm * percent;
		printf("temp_pwm:%.1f\t",temp_pwm);
		printf("temp_pos:%.1f\t",temp_pos);
		printf("percent:%.1f\t",percent);
		printf("pwm:%.1f\t",pwm);
	}
	else
		pwm = vertex[0].pwm_y;
	return pwm;
}
void Topic_2_Draw_screen(void)
{
	/*
	Target1 = vertex[1].pwm_x;	Target2 = vertex[1].pwm_y;
	delay_ms(1000);
	delay_ms(500);
	Target1 = vertex[2].pwm_x;	Target2 = vertex[2].pwm_y;
	delay_ms(1000);
	delay_ms(500);
	Target1 = vertex[3].pwm_x;	Target2 = vertex[3].pwm_y;
	delay_ms(1000);
	delay_ms(500);
	Target1 = vertex[4].pwm_x;	Target2 = vertex[4].pwm_y;
	delay_ms(1000);
	delay_ms(500);
	Target1 = vertex[1].pwm_x;	Target2 = vertex[1].pwm_y;
	delay_ms(1000);
	delay_ms(500);
	Target1 = vertex[0].pwm_x;	Target2 = vertex[0].pwm_y;
	delay_ms(1000);
	Reset_Wheels();
	*/
	
	/*
	Target1 = Position1 + Cx - vertex[1].x; Target2 = Position2 + Cy - vertex[1].y;
	delay_ms(1000);
	delay_ms(1000);
	Target1 = Position1 + Cx - vertex[2].x; Target2 = Position2 + Cy - vertex[2].y;
	delay_ms(1000);
	delay_ms(1000);
	Target1 = Position1 + Cx - vertex[3].x; Target2 = Position2 + Cy - vertex[3].y;
	delay_ms(1000);
	delay_ms(1000);
	Target1 = Position1 + Cx - vertex[4].x; Target2 = Position2 + Cy - vertex[4].y;
	*/
	Target1 = 820; Target2 = 885;
	delay_ms(1000);delay_ms(1000);delay_ms(1000);
	Target1 = 720; Target2 = 870;
	delay_ms(1000);delay_ms(1000);delay_ms(1000);
	Target1 = 720; Target2 = 750;
	delay_ms(1000);delay_ms(1000);delay_ms(1000);
	Target1 = 820; Target2 = 755;
	delay_ms(1000);delay_ms(1000);delay_ms(1000);
	Target1 = 820; Target2 = 885;
	delay_ms(1000);delay_ms(1000);delay_ms(1000);
	Reset_Wheels(Screen_POS[SCREEN_POS_COUNT - 1].x,Screen_POS[SCREEN_POS_COUNT - 1].y);
}

void Topic_2_Draw_Screen_By_POS(void)
{
	int i;
	
	/*
	for(i = 0; i < SCREEN_POS_COUNT;i++)
	{
		printf("x:%d\t%d\r\n",Screen_POS[i].x,Screen_POS[i].y);
		Asix_Control(Screen_POS[i].x,Screen_POS[i].y);
	}
	Asix_Control(Screen_POS[1].x,Screen_POS[1].y);
	*/
	
	for(i = 0; i < SCREEN_POS_COUNT - 1;i++)
	{
		printf("T2 Get Cx Cy is :%d\t%d\r\n",Screen_POS[i].x,Screen_POS[i].y);
		printf("T2 Get Ax Ay is :%d\t%d\r\n",Screen_POS[i].x,Screen_POS[i].y);
		delay_ms(100);
		while(myabs( Cx - Screen_POS[i].x) >= 2 || myabs( Cy - Screen_POS[i].y) >= 2)
		{
			printf("Moveing To P%d Cx:%d Cy:%d ----> Ax:%d Ay:%d T1:%.1f T2:%.1f\r\n",i+1,Cx,Cy,Screen_POS[i].x,Screen_POS[i].y,Target1,Target2);
			if( Cx - Screen_POS[i].x > 0)
			{
				Target1 = Position1 + 3;
			}
			else if( Cx - Screen_POS[i].x < 0)
			{
				Target1 = Position1 - 3;
			}
			
			printf("Moveing To P%d Cx:%d Cy:%d ----> Ax:%d Ay:%d T1:%.1f T2:%.1f\r\n",i+1,Cx,Cy,Screen_POS[i].x,Screen_POS[i].y,Target1,Target2);
			if( Cy - Screen_POS[i].y > 0)
			{
				Target2 = Position2 + 2.5;
			}
			else if( Cy - Screen_POS[i].y < 0)
			{
				Target2 = Position2 - 2.5;
			}			
			delay_ms(10);
		}
	}
	//�ص����
	while(myabs( Cx - Screen_POS[0].x) >= 2 || myabs( Cy - Screen_POS[0].y) >= 2)
	{
		printf("Moveing To P%d Cx:%d Cy:%d ----> Ax:%d Ay:%d T1:%.1f T2:%.1f\r\n",i+1,Cx,Cy,Screen_POS[0].x,Screen_POS[0].y,Target1,Target2);
		if( Cx - Screen_POS[0].x > 0)
		{
			Target1 = Position1 + 2.5;
		}
		else if( Cx - Screen_POS[0].x < 0)
		{
			Target1 = Position1 - 2.5;
		}
			
		printf("Moveing To P%d Cx:%d Cy:%d ----> Ax:%d Ay:%d T1:%.1f T2:%.1f\r\n",i+1,Cx,Cy,Screen_POS[0].x,Screen_POS[0].y,Target1,Target2);
		if( Cy - Screen_POS[0].y > 0)
		{
			Target2 = Position2 + 2.5;
		}
		else if( Cy - Screen_POS[0].y < 0)
		{
			Target2 = Position2 - 2.5;
		}			
		delay_ms(10);
	}
}

//������
//��õ�Ե�֮�������ʼ��ֹ������꣬������PTP_COUNT����
//Ŀ������ptp�ṹ��������
void Get_PTP_POS(int start_X, int start_Y, int end_X, int end_Y)	
{

	int i,j;
	int temp;
	float temp1,temp2;
	temp = (end_X - start_X) / PTP_COUNT;
	for(i = 0; i < PTP_COUNT; i++)
	{
		if(i == 0)
		{
			ptp[i].x = start_X;
			ptp[i].y = start_Y;
			continue;
		}
		if(i == PTP_COUNT - 1)
		{
			ptp[i].x = end_X;
			ptp[i].y = end_Y;
			continue;
		}
		
		ptp[i].x = start_X + i * temp;
	}
	
	temp = (end_Y - start_Y) / PTP_COUNT;
	for(i = 1; i < PTP_COUNT - 1; i++)
	{
		ptp[i].y = start_Y + i * temp;
	}
}

/*
void Get_Screen_POS(int start_X, int start_Y, int end_X, int end_Y)	
{

	int i,j;
	int temp;
	float temp1,temp2;
	temp = (end_X - start_X) / PTP_COUNT;
	for(i = 0; i < PTP_COUNT; i++)
	{
		if(i == 0)
		{
			ptp[i].x = start_X;
			ptp[i].y = start_Y;
			continue;
		}
		if(i == PTP_COUNT - 1)
		{
			ptp[i].x = end_X;
			ptp[i].y = end_Y;
			continue;
		}
		
		ptp[i].x = start_X + i * temp;
	}
	
	temp = (end_Y - start_Y) / PTP_COUNT;
	for(i = 1; i < PTP_COUNT - 1; i++)
	{
		ptp[i].y = start_Y + i * temp;
	}
}
*/

//�����⻮��
void Topic_3_Draw_Rectangle(void)
{
	int i;
	usart3_sendAngleBlock(0x01);
	delay_ms(1000);
	usart3_sendAngleBlock(0x04);
	do
	{
		CAM_Data_handle();
		printf("P1:%d,%d\r\n",rectangle_Pos_1_X,rectangle_Pos_1_Y);
		printf("P2:%d,%d\r\n",rectangle_Pos_2_X,rectangle_Pos_2_Y);
		printf("P3:%d,%d\r\n",rectangle_Pos_3_X,rectangle_Pos_3_Y);
		printf("P4:%d,%d\r\n",rectangle_Pos_4_X,rectangle_Pos_4_Y);
	}while(rectangle_Pos_4_Y == 0);
	printf("P1:%d,%d\r\n",rectangle_Pos_1_X,rectangle_Pos_1_Y);
	printf("P2:%d,%d\r\n",rectangle_Pos_2_X,rectangle_Pos_2_Y);
	printf("P3:%d,%d\r\n",rectangle_Pos_3_X,rectangle_Pos_3_Y);
	printf("P4:%d,%d\r\n",rectangle_Pos_4_X,rectangle_Pos_4_Y);
	usart3_sendAngleBlock(0x03);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	
	Get_PTP_POS(rectangle_Pos_1_X, rectangle_Pos_1_Y, rectangle_Pos_2_X, rectangle_Pos_2_Y);	//��õ�һ���߶�֮�������
	//Target1 = ptp[0].pwm_x;
	//Target2 = ptp[0].pwm_y;
	for(i = 0; i < PTP_COUNT; i++)
	{
		Asix_Control(ptp[i].x,ptp[i].y);
		//Target1 = Pos_To_Screen_PWM_X( ptp[i].x ); Target2 = Pos_To_Screen_PWM_Y( ptp[i].y );
	}
	
	Get_PTP_POS(rectangle_Pos_2_X, rectangle_Pos_2_Y, rectangle_Pos_3_X, rectangle_Pos_3_Y);	//��õڶ����߶�֮�������
	for(i = 0; i < PTP_COUNT; i++)
	{
		Asix_Control(ptp[i].x,ptp[i].y);
		//Target1 = Pos_To_Screen_PWM_X( ptp[i].x ); Target2 = Pos_To_Screen_PWM_Y( ptp[i].y );
	}
	
	Get_PTP_POS(rectangle_Pos_3_X, rectangle_Pos_3_Y, rectangle_Pos_4_X, rectangle_Pos_4_Y);	//��õ������߶�֮�������
	for(i = 0; i < PTP_COUNT; i++)
	{
		Asix_Control(ptp[i].x,ptp[i].y);
		//Target1 = Pos_To_Screen_PWM_X( ptp[i].x ); Target2 = Pos_To_Screen_PWM_Y( ptp[i].y );
	}
	
	Get_PTP_POS(rectangle_Pos_4_X, rectangle_Pos_4_Y, rectangle_Pos_1_X, rectangle_Pos_1_Y);	//��õ��ĸ��߶�֮�������
	for(i = 0; i < PTP_COUNT; i++)
	{
		Asix_Control(ptp[i].x,ptp[i].y);
		//Target1 = Pos_To_Screen_PWM_X( ptp[i].x ); Target2 = Pos_To_Screen_PWM_Y( ptp[i].y );
	}
	
	Reset_Wheels(Screen_POS[SCREEN_POS_COUNT - 1].x,Screen_POS[SCREEN_POS_COUNT - 1].y);
	usart3_sendAngleBlock(0x01);
	
	rectangle_Pos_1_X = 0;//CAM������������
	rectangle_Pos_1_Y = 0;//�����ĸ���������
	rectangle_Pos_2_X = 0;
	rectangle_Pos_2_Y = 0;
	rectangle_Pos_3_X = 0;
	rectangle_Pos_3_Y = 0;
	rectangle_Pos_4_X = 0;
	rectangle_Pos_4_Y = 0;
	
	delay_ms(1000);
	usart3_sendAngleBlock(0X03);
	
}
//������պ���
//����������arr ��ճ���s
void Arr_Clear(char *arr,int len)
{
	int i;
	for(i = 0; i<len; i++)
	{
		*arr = '\0';
		arr++;
	}
}

//���մ���1��Ϣ������Ϣ���浽message_arr��Ϣ����������
void Get_Message(void)
{
	u16 len;
	u16 t;
	if(USART_RX_STA&0x8000)
	{					   
		len=USART_RX_STA&0x3fff;									//�õ��˴ν��յ������ݳ���
		printf("\r\nSTM32 get data:\t");
			
		Arr_Clear(arr,MAX_ARR_LENG);									//�����Ϣ�������½���
			
		for(t=0;t<len;t++)
		{
			USART_SendData(USART1, USART_RX_BUF[t]);				//�򴮿�1��������
			arr[t] = USART_RX_BUF[t];						//����õ���Ϣ���浽message_arr��Ϣ����
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);	//�ȴ����ͽ���
		}
		printf("\r\n");//���뻻��
		USART_RX_STA=0;
	}
	//else
		//printf("No Message");
}

void Message_Handler(void)
{
	int x,y;
	int int_Value;
	float float_Value;
	char temp_Buf[3];
	
	if(arr[0] == '0' || arr[0] == '\0')				//��Ϣ����ͷ�ַ�Ϊ0����������
	{
		Arr_Clear(arr,20);
		return;
	}

	if(arr[0] == 'M')
	{
		if(arr[1] == 'X')
		{	
			if(arr[2]>='0' || arr[2]<='9' )	temp_Buf[0] = arr[2];
			else{printf("���ݴ����Ϲ�ֹͣ����");return;}
			
			if(arr[3]>='0' || arr[3]<='9' )	temp_Buf[1] = arr[3];
			else{printf("���ݴ����Ϲ�ֹͣ����");return;}
			
			if(arr[4]>='0' || arr[4]<='9' )	temp_Buf[2] = arr[4];
			else{printf("���ݴ����Ϲ�ֹͣ����");return;}
			
			x = atoi(temp_Buf);
			//Set_X_Wheel_Angle(x);
			Target1 = x;
			printf("X����ת��%d�Ƚ�",x);
		}
		if(arr[5] == 'Y')
		{	
			if(arr[6]>='0' || arr[6]<='9' )	temp_Buf[0] = arr[6];
			else{printf("���ݴ����Ϲ�ֹͣ����");return;}
			
			if(arr[7]>='0' || arr[7]<='9' )	temp_Buf[1] = arr[7];
			else{printf("���ݴ����Ϲ�ֹͣ����");return;}
			
			if(arr[8]>='0' || arr[8]<='9' )	temp_Buf[2] = arr[8];
			else{printf("���ݴ����Ϲ�ֹͣ����");return;}
			
			y = atoi(temp_Buf);
			//Set_Y_Wheel_Angle(y);
			Target2 = y;
			printf("Y����ת��%d�Ƚ�",y);
		}
		
		if(arr[0] == 0x2C)
		{
			x = arr[2];
			y = arr[3];
			printf("%d\t%d\r\n",x,y);
		}
	}
	if(arr[0] == 'S')								//����ԭ���Լ�4������
	{
		if(arr[1] == '0')
		{
			vertex[0].x = Cx;
			vertex[0].y = Cy;
			vertex[0].pwm_x = Position1;
			vertex[0].pwm_y = Position2;
			printf("Vertex0: %d,%d\t%.1f,%.1f\r\n",Cx,Cy,Position1,Position2);
			}
		if(arr[1] == '1')
		{
			vertex[1].x = Cx;
			vertex[1].y = Cy;
			vertex[1].pwm_x = Position1;
			vertex[1].pwm_y = Position2;
			printf("Vertex1: %d,%d\t%.1f,%.1f\r\n",Cx,Cy,Position1,Position2);
		}
		if(arr[1] == '2')
		{
			vertex[2].x = Cx;
			vertex[2].y = Cy;
			vertex[2].pwm_x = Position1;
			vertex[2].pwm_y = Position2;
			printf("Vertex2: %d,%d\t%.1f,%.1f\r\n",Cx,Cy,Position1,Position2);
		}
		if(arr[1] == '3')
		{
			vertex[3].x = Cx;
			vertex[3].y = Cy;
			vertex[3].pwm_x = Position1;
			vertex[3].pwm_y = Position2;
			printf("Vertex3: %d,%d\t%.1f,%.1f\r\n",Cx,Cy,Position1,Position2);
		}
		if(arr[1] == '4')
		{
			vertex[4].x = Cx;
			vertex[4].y = Cy;
			vertex[4].pwm_x = Position1;
			vertex[4].pwm_y = Position2;
			printf("Vertex4: %d,%d\t%.1f,%.1f\r\n",Cx,Cy,Position1,Position2);
		}
		Arr_Clear(arr,MAX_ARR_LENG);
	}
}

void Input_Handler(void)
{
	if(INPUT0 == 1)
	{
		delay_ms(10);
		if(INPUT0 == 1)
		{
			LED0 = 0;
			mode = 0;
		}
	}
	
	if(INPUT1 == 1)
	{
		delay_ms(10);
		if(INPUT1 == 1)
		{
			LED0 = 0;
			mode = 1;
		}
	}
	if(INPUT2 == 1)
	{
		delay_ms(10);
		if(INPUT2 == 1)
		{
			LED0 = 0;
			mode = 2;
		}
	}
	if(INPUT3 == 1)
	{
		delay_ms(10);
		if(INPUT3 == 1)
		{
			LED0 = 0;
			usart3_sendAngleBlock(0X03);
		}
	}
}



