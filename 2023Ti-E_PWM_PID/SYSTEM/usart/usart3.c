#include "usart3.h"


void My_USART3_Init(u32 bound)  
{  
	GPIO_InitTypeDef GPIO_InitStrue; //����һ�����ų�ʼ���Ľṹ��
	USART_InitTypeDef USART_InitStrue; //����һ�����ڳ�ʼ���Ľṹ��
	NVIC_InitTypeDef NVIC_InitStrue; //����һ���ж����ȼ���ʼ���Ľṹ��
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); //GPIOBʱ��ʹ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); //����3ʱ��ʹ��
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_AF_PP; //B10������Ϊ����3�����������ţ����츴�����
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_10; //����10
	GPIO_InitStrue.GPIO_Speed=GPIO_Speed_10MHz; //��Ϊ���ڷ�����������ʱ���ٶȿ���Ϊ����
  GPIO_Init(GPIOB,&GPIO_InitStrue); //�����������úõ�GPIO_InitStructure�������г�ʼ��
	GPIO_ResetBits(GPIOB, GPIO_Pin_2);
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_IN_FLOATING; //A10������Ϊ����1�����������ţ�������������������
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_11; //����11
	GPIO_InitStrue.GPIO_Speed=GPIO_Speed_10MHz; //��Ϊ���ڽ�����������ʱ���ٶȿ���Ϊ����
  GPIO_Init(GPIOB,&GPIO_InitStrue); //�����������úõ�GPIO_InitStructure�������г�ʼ��
	
	USART_InitStrue.USART_BaudRate=bound; //���崮�ڲ�����Ϊ9600bit/s
	USART_InitStrue.USART_HardwareFlowControl=USART_HardwareFlowControl_None; //��Ӳ������������
	USART_InitStrue.USART_Mode=USART_Mode_Tx|USART_Mode_Rx; //���ͽ��ռ���ģʽ
	USART_InitStrue.USART_Parity=USART_Parity_No; //����żУ��λ
	USART_InitStrue.USART_StopBits=USART_StopBits_1; //һ��ֹͣλ
	USART_InitStrue.USART_WordLength=USART_WordLength_8b; //�ֳ�Ϊ8λ���ݸ�ʽ
	USART_Init(USART3,&USART_InitStrue);//������������USART_InitStrue������ʼ������1
	
	USART_Cmd(USART3,ENABLE); //ʹ�ܴ���1
	
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE); //ʹ�ܽ����ж�void USART1_IRQHandler(void)
	
	NVIC_InitStrue.NVIC_IRQChannel=USART3_IRQn; //���ڴ���3�ж�
	NVIC_InitStrue.NVIC_IRQChannelCmd=ENABLE; //�ж�ʹ��
	NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority=1; //��ռ���ȼ�Ϊ1����ֵԽС���ȼ�Խ�ߣ�0�����ȼ����
	NVIC_InitStrue.NVIC_IRQChannelSubPriority=1; //��Ӧ���ȼ�Ϊ1����ֵԽС���ȼ�Խ�ߣ�0�����ȼ����
	NVIC_Init(&NVIC_InitStrue); ////����NVIC_InitStrue�Ĳ�����ʼ��VIC�Ĵ��������ô���1�ж����ȼ�
}


/**************************************************************************
�������ܣ�����3��������
��ڲ�������
�� �� ֵ����
**************************************************************************/
void usart3_send(u8 data)
{
	USART3->DR = data;
	while((USART3->SR&0x40)==0);	
}

/**************************************************************************
�������ܣ�����2�жϷ�����
��ڲ�������
����  ֵ����
**************************************************************************/
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE)) //���յ�����
	{	         	
		u8 temp;
		static u8 count,last_data,last_last_data, head_received;
		temp=USART3->DR;
		
		if(head_received==0)
		{	
			if(last_data==0x2c)  //�ж�֡ͷ
			{	
				head_received=1;
				count=0;
			}
		}
		if(head_received==1) //���յ�֡ͷ��ʼ���ձ�������
		{	
			Urxbuf[count]=temp;     
			count++;                
			if(count==9)
			{
				head_received=0;
				count=0;
				Usart_Compelet=1; //������һ�����ݱ�־
			}
		}
		CAM_Data_handle();
		last_last_data=last_data;
		last_data=temp;
   }
}

/**************************************************************************
�������ܣ�����3�������ݴ���
��ڲ�������
����  ֵ����
**************************************************************************/
void CAM_Data_handle(void)
{
	if(Usart_Compelet == 1)
	{
		if(Urxbuf[0] == 8)						//����ǽ��յ�8�����ݣ�����CAM�����ĸ���������
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
	usart3_send(order);       //֡ͷ
}
