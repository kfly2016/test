#include "ms56xx.h"
#include "bsp.h"
#include "iic.h"

static u16 PROM_C[MS561101BA_PROM_REG_COUNT]; //读取标定值

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //?? 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //?? 
   	   		   
//SCL-->PB6
//SDA-->PB7
#define SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=0x80000000;}
#define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=0x30000000;}
 
#define IIC_SCL    PBout(6) 		//SCL
#define IIC_SDA    PBout(7) 		//SDA	 
#define READ_SDA   PBin(7) 		//SDA_IN 

/**************************????********************************************
*????:		void IIC_Start(void)
*?  ?:		??IIC????
*******************************************************************************/
void IIC_Start(void)
{
	SDA_OUT();     //sda??? 
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL=0;//??I2C??,????????? 
}

/**************************????********************************************
*????:		void IIC_Stop(void)
*?  ?:	    //??IIC????
*******************************************************************************/	  
void IIC_Stop(void)
{
	SDA_OUT();//sda???
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;//??I2C??????
	delay_us(4);							   	
}

/**************************????********************************************
*????:		u8 IIC_Wait_Ack(void)
*?  ?:	    ???????? 
//???:1,??????
//        0,??????
*******************************************************************************/
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA?????  
	IIC_SDA=1;delay_us(4);	   
	IIC_SCL=1;delay_us(4);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>50)
		{
			IIC_Stop();
			return 1;
		}
	  delay_us(4);
	}
	IIC_SCL=0;//????0 	   
	return 0;  
} 

/**************************????********************************************
*????:		void IIC_Ack(void)
*?  ?:	    ??ACK??
*******************************************************************************/
void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(4);
	IIC_SCL=1;
	delay_us(4);
	IIC_SCL=0;
}
	
/**************************????********************************************
*????:		void IIC_NAck(void)
*?  ?:	    ??NACK??
*******************************************************************************/	    
void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(4);
	IIC_SCL=1;
	delay_us(4);
	IIC_SCL=0;
}					 				     

/**************************????********************************************
*????:		void IIC_Send_Byte(u8 txd)
*?  ?:	    IIC??????
*******************************************************************************/		  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL=0;//??????????
    for(t=0;t<8;t++)
    {              
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(4);   
		IIC_SCL=1;
		delay_us(4); 
		IIC_SCL=0;	
		delay_us(4);
    }	 
} 	 
   
/**************************????********************************************
*????:		u8 IIC_Read_Byte(unsigned char ack)
*?  ?:	    //?1???,ack=1?,??ACK,ack=0,??nACK 
*******************************************************************************/  
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA?????
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0; 
        delay_us(4);
				IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_us(4); 
    }					 
    if (ack)
        IIC_Ack(); //??ACK 
    else
        IIC_NAck();//??nACK  
    return receive;
}

/**************************????********************************************
*????:		unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr)
*?  ?:	    ?????? ?????????
??	I2C_Addr  ??????
		addr	   ?????
??   ?????
*******************************************************************************/ 
unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr)
{
	unsigned char res=0;
	
	IIC_Start();	
  

  
  
	IIC_Send_Byte(I2C_Addr);	   //?????
	res++;
	IIC_Wait_Ack();
	IIC_Send_Byte(addr); res++;  //????
	IIC_Wait_Ack();	  
	//IIC_Stop();//????????	
	IIC_Start();
	IIC_Send_Byte(I2C_Addr+1); res++;          //??????			   
	IIC_Wait_Ack();
	res=IIC_Read_Byte(0);	   
    IIC_Stop();//????????

	return res;
}


/**************************????********************************************
*????:		u8 IICreadBytes(u8 dev, u8 reg, u8 length, u8 *data)
*?  ?:	    ?????? ?????? length??
??	dev  ??????
		reg	  ?????
		length ??????
		*data  ????????????
??   ????????
*******************************************************************************/ 
u8 IICreadBytes(u8 dev, u8 reg, u8 length, u8 *data){
    u8 count = 0;
	u8 temp;
	IIC_Start();
	IIC_Send_Byte(dev);	   //?????
	IIC_Wait_Ack();
	IIC_Send_Byte(reg);   //????
    IIC_Wait_Ack();	  
	IIC_Start();
	IIC_Send_Byte(dev+1);  //??????	
	IIC_Wait_Ack();
	
    for(count=0;count<length;count++){
		 
		 if(count!=(length-1))
		 	temp = IIC_Read_Byte(1);  //?ACK????
		 	else  
			temp = IIC_Read_Byte(0);	 //??????NACK

		data[count] = temp;
	}
    IIC_Stop();//????????
    return count;
}


static void ms56xx_reset(void) 
{
	IIC_Start();
  IIC_Send_Byte(MS5611_ADDR); //???
	IIC_Wait_Ack();
  IIC_Send_Byte(MS561101BA_RESET);//??????
	IIC_Wait_Ack();	
  IIC_Stop();
}

static void ms56xx_readPROM(void) 
{
	u8  int_h,int_l;
	u8  i = 0;
	
	for (i=0; i < MS561101BA_PROM_REG_COUNT; i++) 
	{
			IIC_Start();
			IIC_Send_Byte(MS5611_ADDR);
			IIC_Wait_Ack();
			IIC_Send_Byte(0XA0 + (i * MS561101BA_PROM_REG_SIZE));
			IIC_Wait_Ack();	
			IIC_Stop();
			delay_us(5);
			IIC_Start();
			IIC_Send_Byte(MS5611_ADDR+1);  
			delay_us(3);
			IIC_Wait_Ack();
			int_h = IIC_Read_Byte(1);  //?ACK????
			delay_us(3);
			int_l = IIC_Read_Byte(0);	 //??????NACK
			IIC_Stop();
			
			PROM_C[i] = (((u16)int_h << 8) | int_l);
	}
}

void ms56xx_startConversion(u8 cmd) 
{
	// initialize pressure conversion
	IIC_Start();
	IIC_Send_Byte(MS5611_ADDR); //???
	IIC_Wait_Ack();
	IIC_Send_Byte(cmd); //?????
	IIC_Wait_Ack();	
	IIC_Stop();
}

static u32 ms56xx_getConversion(void) 
{
	u32 conversion = 0;
	u8 temp[3];
	
	IIC_Start();
	IIC_Send_Byte(MS5611_ADDR); //???
	IIC_Wait_Ack();
	IIC_Send_Byte(0);// start read sequence
	IIC_Wait_Ack();	
	IIC_Stop();
	
	IIC_Start();
	IIC_Send_Byte(MS5611_ADDR+1);  //??????	
	IIC_Wait_Ack();
	
	temp[0] = IIC_Read_Byte(1);  //?ACK????  bit 23-16
	temp[1] = IIC_Read_Byte(1);  //?ACK????  bit 8-15
	temp[2] = IIC_Read_Byte(0);  //?NACK???? bit 0-7
	
	IIC_Stop();
	
	conversion = (unsigned long)temp[0] * 65536 + (unsigned long)temp[1] * 256 + (unsigned long)temp[2];
	
	return conversion;
}


float ms56xx_getTemperature(void)
{	
	u32 ret_temp = 0; 
	float f_temp = 0.0 ;
	
	ret_temp = ms56xx_getConversion();
  f_temp = ret_temp - (((unsigned long)PROM_C[5]) << 8);
	
	return (2000 + f_temp*(((unsigned long)PROM_C[6]) << 8));
}

void ms56xx_init(void) 
{  
	ms56xx_reset(); // ?? MS561101B 
	delay_ms(100); // ?? 
	ms56xx_readPROM(); // ??EEPROM ????? ??	
}

