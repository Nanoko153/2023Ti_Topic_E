#include "usart3.h"


void My_USART3_Init(u32 bound)  
{  
	GPIO_InitTypeDef GPIO_InitStrue; //定义一个引脚初始化的结构体
	USART_InitTypeDef USART_InitStrue; //定义一个串口初始化的结构体
	NVIC_InitTypeDef NVIC_InitStrue; //定义一个中断优先级初始化的结构体
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); //GPIOB时钟使能
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); //串口3时钟使能
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_AF_PP; //B10引脚作为串口3发送数据引脚，推挽复用输出
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_10; //引脚10
	GPIO_InitStrue.GPIO_Speed=GPIO_Speed_10MHz; //作为串口发送数据引脚时该速度可以为任意
  GPIO_Init(GPIOB,&GPIO_InitStrue); //根据上面设置好的GPIO_InitStructure参数进行初始化
	GPIO_ResetBits(GPIOB, GPIO_Pin_2);
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_IN_FLOATING; //A10引脚作为串口1接收数据引脚，浮空输入或带上拉输入
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_11; //引脚11
	GPIO_InitStrue.GPIO_Speed=GPIO_Speed_10MHz; //作为串口接收数据引脚时该速度可以为任意
  GPIO_Init(GPIOB,&GPIO_InitStrue); //根据上面设置好的GPIO_InitStructure参数进行初始化
	
	USART_InitStrue.USART_BaudRate=bound; //定义串口波特率为9600bit/s
	USART_InitStrue.USART_HardwareFlowControl=USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStrue.USART_Mode=USART_Mode_Tx|USART_Mode_Rx; //发送接收兼容模式
	USART_InitStrue.USART_Parity=USART_Parity_No; //无奇偶校验位
	USART_InitStrue.USART_StopBits=USART_StopBits_1; //一个停止位
	USART_InitStrue.USART_WordLength=USART_WordLength_8b; //字长为8位数据格式
	USART_Init(USART3,&USART_InitStrue);//根据上面设置USART_InitStrue参数初始化串口1
	
	USART_Cmd(USART3,ENABLE); //使能串口1
	
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE); //使能接收中断void USART1_IRQHandler(void)
	
	NVIC_InitStrue.NVIC_IRQChannel=USART3_IRQn; //属于串口3中断
	NVIC_InitStrue.NVIC_IRQChannelCmd=ENABLE; //中断使能
	NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority=1; //抢占优先级为1级，值越小优先级越高，0级优先级最高
	NVIC_InitStrue.NVIC_IRQChannelSubPriority=1; //响应优先级为1级，值越小优先级越高，0级优先级最高
	NVIC_Init(&NVIC_InitStrue); ////根据NVIC_InitStrue的参数初始化VIC寄存器，设置串口1中断优先级
}


/**************************************************************************
函数功能：串口3发送数据
入口参数：无
返 回 值：无
**************************************************************************/
void usart3_send(u8 data)
{
	USART3->DR = data;
	while((USART3->SR&0x40)==0);	
}

/**************************************************************************
函数功能：串口2中断服务函数
入口参数：无
返回  值：无
**************************************************************************/
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE)) //接收到数据
	{	         	
		u8 temp;
		static u8 count,last_data,last_last_data, head_received;
		temp=USART3->DR;
		
		if(head_received==0)
		{	
			if(last_data==0x2c)  //判断帧头
			{	
				head_received=1;
				count=0;
			}
		}
		if(head_received==1) //接收到帧头则开始接收保存数据
		{	
			Urxbuf[count]=temp;     
			count++;                
			if(count==9)
			{
				head_received=0;
				count=0;
				Usart_Compelet=1; //接收完一组数据标志
			}
		}
		CAM_Data_handle();
		last_last_data=last_data;
		last_data=temp;
   }
}

/**************************************************************************
函数功能：串口3接收数据处理
入口参数：无
返回  值：无
**************************************************************************/
void CAM_Data_handle(void)
{
	if(Usart_Compelet == 1)
	{
		if(Urxbuf[0] == 8)						//如果是接收到8个数据，就是CAM返回四个顶点坐标
		{
			rectangle_Pos_1_X = Urxbuf[1];
			rectangle_Pos_1_Y = Urxbuf[2];
			rectangle_Pos_2_X = Urxbuf[3];
			rectangle_Pos_2_Y = Urxbuf[4];
			rectangle_Pos_3_X = Urxbuf[5];
			rectangle_Pos_3_Y = Urxbuf[6];
			rectangle_Pos_4_X = Urxbuf[7];
			rectangle_Pos_4_Y = Urxbuf[8];
		}
		
		if(Urxbuf[0] == 2)
		{
			Cx = Urxbuf[1];
			Cy = Urxbuf[2];
		}
		
		Usart_Compelet = 0;
	}
}

void usart3_sendAngleBlock(char order)
{
	usart3_send(order);       //帧头
}
