#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"

#define BAUND_FREQ  	115200    //ËÙÂÊ115200bps
#define BUFFER_SIZE   50


typedef struct
{
  u8  tx_buf[BUFFER_SIZE]; 
  u8  rx_buf[BUFFER_SIZE];
	u8  tx_cnt;
	u8  rx_cnt;
}Uart_Buf;



void uart_init(void);
void uart_send_char(u8 );
void uart_send_buf(unsigned char *dataToSend , u8 data_num);

#endif

