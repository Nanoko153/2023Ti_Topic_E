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

//宏定义
#define MAX_ARR_LENG 20		//串口1最大接收数据缓存
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

//---------------------------------------------------------------------------------------------//函数定义
//第一题
void Get_Screen_POS(void);			//获得屏幕范围点

//第二题		//沿屏幕移动
float Pos_To_Screen_PWM_X(int pos);
float Pos_To_Screen_PWM_Y(int pos);
void Vertex_Init(void);
void Topic_2_Draw_screen(void);
void Topic_2_Draw_Screen_By_POS(void);

int vertet_count = 0;
struct POS_PWM vertex[5]; 
struct POS_PWM Screen_POS[SCREEN_POS_COUNT];

 

//第三题
void Get_PTP_POS(int start_X, int start_Y, int end_X, int end_Y);
void Topic_3_Draw_Rectangle(void);
struct POS_PWM ptp[PTP_COUNT];
int rectangle_Pos_1_X;//CAM返回坐标数据
int rectangle_Pos_1_Y;//矩形四个顶点坐标
int rectangle_Pos_2_X;
int rectangle_Pos_2_Y;
int rectangle_Pos_3_X;
int rectangle_Pos_3_Y;
int rectangle_Pos_4_X;
int rectangle_Pos_4_Y;


//串口1
u8 str[20];					//字符串缓存
char arr[MAX_ARR_LENG];					//消息数组缓存

//串口3
uint8_t Urxbuf[9]; //串口3接收数据数组
int Usart_Compelet; //串口接收完一组数据标志

//PWM直控（已弃用）
int w_X_PWM;
int w_Y_PWM;

//PID控制
float speed;									//总速度
float speed_x;									//x舵机速度
float speed_y;									//y轴舵机速度
float Position_KP,Position_KI,Position_KD;  	//位置控制PID参数
float Velocity1,Velocity2;    				 	//PWM变量
float Position1,Position2;   					//舵机当前PWM
float Target1,Target2;							//舵机目标PWM

float min_X_PWM = 500;		//第二题范围
float max_X_PWM = 1000;
float min_Y_PWM = 500;
float max_Y_PWM = 1000;

int  Cx,Cy,Cw,Ch;	//当前x，y值

int next_Pos_X;
int next_POS_Y;

int mode = 99;		//模式标志位
int reset_Key;
int can_Get_Pos;
int main(void)
{
	int i;
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);
	My_USART3_Init(115200);
	
	LED_GPIO_Init();
	Input_GPIO_Init();
	
	Wheel_Value_Init();
	
	TIM3_Int_Init(100-1,7200-1);
	TIM2_PWM_Init(9999, 143); //TIM2_Int_Init(u16 arr, u16 psc)，初始化定时器TIM2
	                        //定时时间=（arr+1)(psc+1)/Tclk，Tclk为内部通用定时器时钟，本例程默认设置为72MHZ
	
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
//初始化获取坐标点
void Get_Screen_POS(void)
{
	int i,j;
	//LASER = 1;		//打开激光
	
	while(Cx == 0 && Cy == 0)
	{
		usart3_sendAngleBlock(0X03);		//发送打开坐标请求
		delay_ms(500);						//半秒的等待时间
	}
	//Reset_Wheels();							//重置回坐标中心点
	//delay_ms(1000);							//等待2s激光回到坐标中心点
	//delay_ms(1000);
	//Screen_POS[0].x = Cx;	Screen_POS[0].y = Cy;	//将中心点返回给Screen_POS数组中的0号位
	//LASER = 0; 								//关闭激光
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
	for(i = 0; i < SCREEN_POS_COUNT ;i++)	//开始给定屏幕限定坐标
	{
		while(!can_Get_Pos)
		{
			if(INPUT0 == 1)
			{
				delay_ms(10);
				if(INPUT0 == 1)
				{
					while(INPUT0 == 1);			//等待松手
					can_Get_Pos = 1;			//可写入坐标点标志位
					LED0 = 0;
				}
			}
		}
		Screen_POS[i].x = Cx; Screen_POS[i].y = Cy; //获得第二个坐标
		can_Get_Pos = 0;							//关闭坐标读取等待下一次读取标志
		printf("P%d:x_%d\ty_%d\r\n",i,Screen_POS[i].x,Screen_POS[i].y);
		for(j = 0;j < i; j++)	//闪烁相应次数
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
//第一题 复位


//第二题
float Pos_To_Screen_PWM_X(int pos)
{
	float pwm;
	float temp_pwm;
	float temp_pos;
	float percent;
	if(pos < vertex[0].x)
	{
		temp_pwm = (( vertex[1].pwm_x - vertex[0].pwm_x)+(vertex[4].pwm_x - vertex[0].pwm_x))/2; //获得屏幕左半边的平均PWM差值
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
		temp_pwm = (( vertex[0].pwm_x - vertex[2].pwm_x)+(vertex[0].pwm_x - vertex[3].pwm_x))/2; //获得屏幕右半边的平均PWM差值
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
		temp_pwm = (( vertex[0].pwm_y - vertex[3].pwm_y)+(vertex[0].pwm_y - vertex[4].pwm_y))/2; //获得屏幕左半边的平均PWM差值
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
		temp_pwm = (( vertex[1].pwm_y - vertex[0].pwm_y)+(vertex[2].pwm_y - vertex[0].pwm_y))/2; //获得屏幕右半边的平均PWM差值
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
	//回到起点
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

//第三题
//获得点对点之间包括起始终止点的坐标，数量由PTP_COUNT控制
//目标存放于ptp结构体数组中
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

//第三题划线
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
	
	Get_PTP_POS(rectangle_Pos_1_X, rectangle_Pos_1_Y, rectangle_Pos_2_X, rectangle_Pos_2_Y);	//获得第一个线段之间的坐标
	//Target1 = ptp[0].pwm_x;
	//Target2 = ptp[0].pwm_y;
	for(i = 0; i < PTP_COUNT; i++)
	{
		Asix_Control(ptp[i].x,ptp[i].y);
		//Target1 = Pos_To_Screen_PWM_X( ptp[i].x ); Target2 = Pos_To_Screen_PWM_Y( ptp[i].y );
	}
	
	Get_PTP_POS(rectangle_Pos_2_X, rectangle_Pos_2_Y, rectangle_Pos_3_X, rectangle_Pos_3_Y);	//获得第二个线段之间的坐标
	for(i = 0; i < PTP_COUNT; i++)
	{
		Asix_Control(ptp[i].x,ptp[i].y);
		//Target1 = Pos_To_Screen_PWM_X( ptp[i].x ); Target2 = Pos_To_Screen_PWM_Y( ptp[i].y );
	}
	
	Get_PTP_POS(rectangle_Pos_3_X, rectangle_Pos_3_Y, rectangle_Pos_4_X, rectangle_Pos_4_Y);	//获得第三个线段之间的坐标
	for(i = 0; i < PTP_COUNT; i++)
	{
		Asix_Control(ptp[i].x,ptp[i].y);
		//Target1 = Pos_To_Screen_PWM_X( ptp[i].x ); Target2 = Pos_To_Screen_PWM_Y( ptp[i].y );
	}
	
	Get_PTP_POS(rectangle_Pos_4_X, rectangle_Pos_4_Y, rectangle_Pos_1_X, rectangle_Pos_1_Y);	//获得第四个线段之间的坐标
	for(i = 0; i < PTP_COUNT; i++)
	{
		Asix_Control(ptp[i].x,ptp[i].y);
		//Target1 = Pos_To_Screen_PWM_X( ptp[i].x ); Target2 = Pos_To_Screen_PWM_Y( ptp[i].y );
	}
	
	Reset_Wheels(Screen_POS[SCREEN_POS_COUNT - 1].x,Screen_POS[SCREEN_POS_COUNT - 1].y);
	usart3_sendAngleBlock(0x01);
	
	rectangle_Pos_1_X = 0;//CAM返回坐标数据
	rectangle_Pos_1_Y = 0;//矩形四个顶点坐标
	rectangle_Pos_2_X = 0;
	rectangle_Pos_2_Y = 0;
	rectangle_Pos_3_X = 0;
	rectangle_Pos_3_Y = 0;
	rectangle_Pos_4_X = 0;
	rectangle_Pos_4_Y = 0;
	
	delay_ms(1000);
	usart3_sendAngleBlock(0X03);
	
}
//数组清空函数
//参数：数组arr 清空长度s
void Arr_Clear(char *arr,int len)
{
	int i;
	for(i = 0; i<len; i++)
	{
		*arr = '\0';
		arr++;
	}
}

//接收串口1消息并将消息缓存到message_arr消息缓存数组中
void Get_Message(void)
{
	u16 len;
	u16 t;
	if(USART_RX_STA&0x8000)
	{					   
		len=USART_RX_STA&0x3fff;									//得到此次接收到的数据长度
		printf("\r\nSTM32 get data:\t");
			
		Arr_Clear(arr,MAX_ARR_LENG);									//清除消息缓存重新接收
			
		for(t=0;t<len;t++)
		{
			USART_SendData(USART1, USART_RX_BUF[t]);				//向串口1发送数据
			arr[t] = USART_RX_BUF[t];						//将获得的信息缓存到message_arr消息数组
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);	//等待发送结束
		}
		printf("\r\n");//插入换行
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
	
	if(arr[0] == '0' || arr[0] == '\0')				//消息缓存头字符为0不处理数据
	{
		Arr_Clear(arr,20);
		return;
	}

	if(arr[0] == 'M')
	{
		if(arr[1] == 'X')
		{	
			if(arr[2]>='0' || arr[2]<='9' )	temp_Buf[0] = arr[2];
			else{printf("数据串不合规停止接收");return;}
			
			if(arr[3]>='0' || arr[3]<='9' )	temp_Buf[1] = arr[3];
			else{printf("数据串不合规停止接收");return;}
			
			if(arr[4]>='0' || arr[4]<='9' )	temp_Buf[2] = arr[4];
			else{printf("数据串不合规停止接收");return;}
			
			x = atoi(temp_Buf);
			//Set_X_Wheel_Angle(x);
			Target1 = x;
			printf("X轴舵机转向%d度角",x);
		}
		if(arr[5] == 'Y')
		{	
			if(arr[6]>='0' || arr[6]<='9' )	temp_Buf[0] = arr[6];
			else{printf("数据串不合规停止接收");return;}
			
			if(arr[7]>='0' || arr[7]<='9' )	temp_Buf[1] = arr[7];
			else{printf("数据串不合规停止接收");return;}
			
			if(arr[8]>='0' || arr[8]<='9' )	temp_Buf[2] = arr[8];
			else{printf("数据串不合规停止接收");return;}
			
			y = atoi(temp_Buf);
			//Set_Y_Wheel_Angle(y);
			Target2 = y;
			printf("Y轴舵机转向%d度角",y);
		}
		
		if(arr[0] == 0x2C)
		{
			x = arr[2];
			y = arr[3];
			printf("%d\t%d\r\n",x,y);
		}
	}
	if(arr[0] == 'S')								//设置原点以及4个顶点
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



