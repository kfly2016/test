#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"

//IO∫Í∂®“Â
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))  

//IO”≥…‰
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 


#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //?? 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //?? 
   	   		   
//SCL-->PB6
//SDA-->PB7
#define MPU_SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=0x80000000;}
#define MPU_SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=0x30000000;}
 
#define MPU_IIC_SCL    PBout(6) 		//SCL
#define MPU_IIC_SDA    PBout(7) 		//SDA	 
#define MPU_READ_SDA   PBin(7) 		//SDA_IN 

//IIC??????
void MPU_IIC_Delay(void);				//MPU IIC????
void MPU_IIC_Init(void);                //???IIC?IO?				 
void MPU_IIC_Start(void);				//??IIC????
void MPU_IIC_Stop(void);	  			//??IIC????
void MPU_IIC_Send_Byte(u8 txd);			//IIC??????
u8 MPU_IIC_Read_Byte(unsigned char ack);//IIC??????
u8 MPU_IIC_Wait_Ack(void); 				//IIC??ACK??
void MPU_IIC_Ack(void);					//IIC??ACK??
void MPU_IIC_NAck(void);				//IIC???ACK??

void IMPU_IC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 MPU_IIC_Read_One_Byte(u8 daddr,u8 addr);

#endif
