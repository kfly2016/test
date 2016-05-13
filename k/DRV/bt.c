#include "stm32f10x.h"
#include "stdio.h"
#include "uart.h"



/********************************************************************
函数功能：蓝牙使能
入口参数：无。
返    回：无。
备    注：PB2
********************************************************************/
void bt_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //打开外设PORTB的时钟
	  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_2);
	
	//printf("-> bt enable...\r\n");
}

/********************************************************************
函数功能：蓝牙发送指令
入口参数：无。
返    回：无。
备    注：
********************************************************************/
void bt_send_cmd(u8 fun, u8*data, u8 len)
{
  u8 send_buf[32];
	u8 i = 0;
	
	if(len>28)
		return;	//最多28个字节
	
	send_buf[len+3]=0;	//校验数为0
	send_buf[0]=0x88;	//帧头
	send_buf[1]=fun;	//功能字
	send_buf[2]=len;	//数据长度
	
	for(i=0;i<len;i++)
	   send_buf[3+i]=data[i];			//复制数据
	
	for(i=0;i<len+3;i++)
	   send_buf[len+3]+=send_buf[i];	//计算校验和	
	
	for(i=0;i<len+4;i++)
		uart_send_char(send_buf[i]);	//发送
}

/********************************************************************
函数功能：蓝牙设置
入口参数：无。
返    回：无。
备    注：
********************************************************************/
void bt_set_parm(void)
{
	const char ATcmdNameSet[] = 	 {"AT+NAMEkfly\r\n"};    //设置蓝牙设备名
  const char ATcmdBaudSet[] =    {"AT+BAUD8\r\n"};
	const char *p;
	static u8 i = 0;
	
	p = ATcmdNameSet;
  for(i=0; i<strlen(p); i++)
    uart_send_char(*(p+i)); 
	
	delay_ms(500);//写完一条指令，延时500ms再度接收缓存

  p = ATcmdBaudSet;
  for(i=0; i<strlen(p); i++)
    uart_send_char(*(p+i)); 	
}
