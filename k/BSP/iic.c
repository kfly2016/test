#include "stm32f10x.h"
#include "stdio.h"
#include "iic.h"
#include "bsp.h"

void MPU_IIC_Init(void)
{			
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	
 	//配置PB6 PB7 为开漏输出  刷新频率为10Mhz
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
	//应用配置到GPIOB 
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
	//printf("-> iic bus enable...\r\n");
}

//MPU IIC ????
void MPU_IIC_Delay(void)
{
	delay_us(2);//delay_us(2);
}

//??IIC????
void MPU_IIC_Start(void)
{
	MPU_SDA_OUT();     //sda???
	MPU_IIC_SDA=1;	  	  
	MPU_IIC_SCL=1;
	MPU_IIC_Delay();
 	MPU_IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	MPU_IIC_Delay();
	MPU_IIC_SCL=0;//??I2C??,????????? 
}

//??IIC????
void MPU_IIC_Stop(void)
{
	MPU_SDA_OUT();//sda???
	MPU_IIC_SCL=0;
	MPU_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	MPU_IIC_Delay();
	MPU_IIC_SCL=1; 
	MPU_IIC_SDA=1;//??I2C??????
	MPU_IIC_Delay();							   	
}

//????????
//???:1,??????
//        0,??????
u8 MPU_IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	
	MPU_SDA_IN();      //SDA?????  
	MPU_IIC_SDA=1;MPU_IIC_Delay();	   
	MPU_IIC_SCL=1;MPU_IIC_Delay();	 
	while(MPU_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			MPU_IIC_Stop();
			return 1;
		}
	}
	
	MPU_IIC_SCL=0;//????0 	
	
	return 0;  
} 

//??ACK??
void MPU_IIC_Ack(void)
{
	MPU_IIC_SCL=0;
	MPU_SDA_OUT();
	MPU_IIC_SDA=0;
	MPU_IIC_Delay();
	MPU_IIC_SCL=1;
	MPU_IIC_Delay();
	MPU_IIC_SCL=0;
}

//???ACK??		    
void MPU_IIC_NAck(void)
{
	MPU_IIC_SCL=0;
	MPU_SDA_OUT();
	MPU_IIC_SDA=1;
	MPU_IIC_Delay();
	MPU_IIC_SCL=1;
	MPU_IIC_Delay();
	MPU_IIC_SCL=0;
}

//IIC??????
//????????
//1,???
//0,???			  
void MPU_IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	
	  MPU_SDA_OUT(); 	    
    MPU_IIC_SCL=0;//??????????
    for(t=0;t<8;t++)
    {              
       MPU_IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		    MPU_IIC_SCL=1;
		MPU_IIC_Delay(); 
		MPU_IIC_SCL=0;	
		MPU_IIC_Delay();
    }	 
} 

//?1???,ack=1?,??ACK,ack=0,??nACK   
u8 MPU_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	
	MPU_SDA_IN();//SDA?????
    for(i=0;i<8;i++ )
	{
        MPU_IIC_SCL=0; 
        MPU_IIC_Delay();
		MPU_IIC_SCL=1;
        receive<<=1;
        if(MPU_READ_SDA)receive++;   
		MPU_IIC_Delay(); 
    }					 
    if (!ack)
        MPU_IIC_NAck();//??nACK
    else
        MPU_IIC_Ack(); //??ACK   
    return receive;
}