#include "stm32f10x.h"
#include "stdio.h"
#include "uart.h"
#include "com.h"

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef�� d in stdio.h. */ 
FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif 
////////////////////////////////////////////////////////////////////

Uart_Buf uart_buf;  //��ʼ��һ��uart_buf
volatile u8 tmp_cnt = 0;
volatile u8 RxState = 0;

/********************************************************************
�������ܣ���ʼ��UART1
��ڲ���: ��
��    �أ��ޡ�
��    ע��������115200
********************************************************************/
void uart_init(void)
{  	 
	//GPIO????
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//??USART1,GPIOA??

  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//??????
  GPIO_Init(GPIOA, &GPIO_InitStructure);//???GPIOA.9
   
  //USART1_RX	  GPIOA.10???
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//????
  GPIO_Init(GPIOA, &GPIO_InitStructure);//???GPIOA.10  

  //Usart1 NVIC ??
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//?????3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//????3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ????
	NVIC_Init(&NVIC_InitStructure);	//??????????VIC???
  
   //USART ?????

	USART_InitStructure.USART_BaudRate = BAUND_FREQ;//?????
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//???8?????
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//?????
	USART_InitStructure.USART_Parity = USART_Parity_No;//??????
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//????????
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//????

  USART_Init(USART1, &USART_InitStructure); //?????1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//????????
  USART_Cmd(USART1, ENABLE);                    //????1 
	
	uart_buf.rx_cnt = 0;
	uart_buf.tx_cnt = 0;
}

void USART1_IRQHandler(void)
{
	u8 com_data ;
	static u8 _data_len = 0;
	
	if(USART1->SR & USART_SR_ORE)//ORE�ж�
	{
		com_data = USART1->DR;//USART_ClearFlag(USART1,USART_IT_ORE);
	}
	
	//�����ж� (���ռĴ����ǿ�) /////////////////////////////////////////////////////////////////////////////////////////
	if(USART1->SR & (1<<5))//if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //   
	{
		com_data = USART1->DR;
		
		if(RxState==0&&com_data==0xAA)
		{
			RxState = 1;
			uart_buf.rx_buf[0]=com_data;
		}
		else if(RxState==1&&com_data==0xAF)
		{
			RxState=2;
			uart_buf.rx_buf[1]=com_data;
		}
		else if(RxState==2&&com_data>0&&com_data<0XF1)
		{
			RxState=3;
			uart_buf.rx_buf[2]=com_data;
		}
		else if(RxState==3&&com_data<50)
		{
			RxState = 4;
			uart_buf.rx_buf[3]=com_data;
			_data_len = com_data;
			uart_buf.rx_cnt = 0;
		}
		else if(RxState==4&&_data_len>0)
		{
			_data_len--;
			uart_buf.rx_buf[4 + uart_buf.rx_cnt++]=com_data;
			if(_data_len==0)
				RxState = 5;
		}
		else if(RxState==5)
		{
			RxState = 0;
			uart_buf.rx_buf[4 + uart_buf.rx_cnt]=com_data;
			com_recv_data(uart_buf.rx_buf, uart_buf.rx_cnt + 5);
		}
		else
			RxState = 0;
	}
	/*
  //�����ж�
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  
  {
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);//��������жϱ�־
		
		uart_buf.rx_buf[uart_buf.rx_cnt] = USART_ReceiveData(USART1);	//���շŵ���ʱ������
		if(uart_buf.rx_cnt > 50) 
			uart_buf.rx_cnt = 0;
  }
	*/
	//�����ж�
	if((USART1->SR & (1<<7))&&(USART1->CR1 & USART_CR1_TXEIE))//if(USART_GetITStatus(USART1,USART_IT_TXE)!=RESET)
	{
		USART1->DR = uart_buf.tx_buf[uart_buf.tx_cnt++]; //?DR??????          
		if(uart_buf.tx_cnt == tmp_cnt)
		{
			tmp_cnt = 0;
			USART1->CR1 &= ~USART_CR1_TXEIE; //USART_ITConfig(USART1,USART_IT_TXE,DISABLE);//		//??TXE??
		}
	}
}

void uart_send_char(u8 tmp_data)
{
  while((USART1->SR&0x40)==0); //�ȴ��������  
	USART1->DR = tmp_data;   
}

void uart_send_buf(unsigned char *dataToSend , u8 data_num)
{
	u8 i = 0;
	
	for(i = 0; i < data_num; i ++)
	{
		uart_buf.tx_buf[tmp_cnt++] = *(dataToSend + i);
	}
	
	if(!(USART1->CR1 & USART_CR1_TXEIE))
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE); 
}
