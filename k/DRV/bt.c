#include "stm32f10x.h"
#include "stdio.h"
#include "uart.h"



/********************************************************************
�������ܣ�����ʹ��
��ڲ������ޡ�
��    �أ��ޡ�
��    ע��PB2
********************************************************************/
void bt_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //������PORTB��ʱ��
	  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_2);
	
	//printf("-> bt enable...\r\n");
}

/********************************************************************
�������ܣ���������ָ��
��ڲ������ޡ�
��    �أ��ޡ�
��    ע��
********************************************************************/
void bt_send_cmd(u8 fun, u8*data, u8 len)
{
  u8 send_buf[32];
	u8 i = 0;
	
	if(len>28)
		return;	//���28���ֽ�
	
	send_buf[len+3]=0;	//У����Ϊ0
	send_buf[0]=0x88;	//֡ͷ
	send_buf[1]=fun;	//������
	send_buf[2]=len;	//���ݳ���
	
	for(i=0;i<len;i++)
	   send_buf[3+i]=data[i];			//��������
	
	for(i=0;i<len+3;i++)
	   send_buf[len+3]+=send_buf[i];	//����У���	
	
	for(i=0;i<len+4;i++)
		uart_send_char(send_buf[i]);	//����
}

/********************************************************************
�������ܣ���������
��ڲ������ޡ�
��    �أ��ޡ�
��    ע��
********************************************************************/
void bt_set_parm(void)
{
	const char ATcmdNameSet[] = 	 {"AT+NAMEkfly\r\n"};    //���������豸��
  const char ATcmdBaudSet[] =    {"AT+BAUD8\r\n"};
	const char *p;
	static u8 i = 0;
	
	p = ATcmdNameSet;
  for(i=0; i<strlen(p); i++)
    uart_send_char(*(p+i)); 
	
	delay_ms(500);//д��һ��ָ���ʱ500ms�ٶȽ��ջ���

  p = ATcmdBaudSet;
  for(i=0; i<strlen(p); i++)
    uart_send_char(*(p+i)); 	
}
