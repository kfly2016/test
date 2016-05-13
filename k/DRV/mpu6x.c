#include "stm32f10x.h"
#include "stdio.h"
#include "iic.h"
#include "mpu6x.h"
#include "bsp.h"
#include "typeconf.h"
#include "led.h"

//???MPU6050

u8 MPU_Init(void)
{ 
	u8 res;
	//RCC->APB2ENR|=1<<2;     //??PORTA?? 
	//GPIOA->CRH&=0X0FFFFFFF;	//PA15???????	  
	//GPIOA->CRH|=0X30000000; 
	//JTAG_Set(SWD_ENABLE);	//??JTAG,??PA15?????IO??,??PA15?????IO!!!
	//MPU_AD0_CTRL=0;			//??MPU6050?AD0?????,?????:0X68
	
	MPU_IIC_Init();//???IIC??
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X80);	//??MPU6050
  delay_ms(100);
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X00);	//??MPU6050 
	MPU_Set_Gyro_Fsr(3);					//??????,Б2000dps
	MPU_Set_Accel_Fsr(0);					//??????,Б2g
	MPU_Set_Rate(50);						//?????50Hz
	MPU_Write_Byte(MPU_INT_EN_REG,0X00);	//??????
	MPU_Write_Byte(MPU_USER_CTRL_REG,0X00);	//I2C?????
	MPU_Write_Byte(MPU_FIFO_EN_REG,0X00);	//??FIFO
	MPU_Write_Byte(MPU_INTBP_CFG_REG,0X82);	//MPU_INTBP_CFG_REG 0x37
	res=MPU_Read_Byte(MPU_DEVICE_ID_REG);
	if(res==MPU_ADDR)//??ID??
	{
		MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X01);	//??CLKSEL,PLL X????
		MPU_Write_Byte(MPU_PWR_MGMT2_REG,0X00);	//??????????
		MPU_Set_Rate(50);						//??????50Hz
 	}
	else 
		return 1;
	
	return 0;
}
//??MPU6050???????????
//fsr:0,Б250dps;1,Б500dps;2,Б1000dps;3,Б2000dps
//???:0,????
//    ??,???? 
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
	return MPU_Write_Byte(MPU_GYRO_CFG_REG,fsr<<3);//??????????  
}
//??MPU6050???????????
//fsr:0,Б2g;1,Б4g;2,Б8g;3,Б16g
//???:0,????
//    ??,???? 
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
	return MPU_Write_Byte(MPU_ACCEL_CFG_REG,fsr<<3);//?????????????  
}
//??MPU6050????????
//lpf:????????(Hz)
//???:0,????
//    ??,???? 
u8 MPU_Set_LPF(u16 lpf)
{
	u8 data=0;
	if(lpf>=188)data=1;
	else if(lpf>=98)data=2;
	else if(lpf>=42)data=3;
	else if(lpf>=20)data=4;
	else if(lpf>=10)data=5;
	else data=6; 
	return MPU_Write_Byte(MPU_CFG_REG,data);//?????????  
}
//??MPU6050????(??Fs=1KHz)
//rate:4~1000(Hz)
//???:0,????
//    ??,???? 
u8 MPU_Set_Rate(u16 rate)
{
	u8 data;
	if(rate>1000)rate=1000;
	if(rate<4)rate=4;
	data=1000/rate-1;
	data=MPU_Write_Byte(MPU_SAMPLE_RATE_REG,data);	//?????????
 	return MPU_Set_LPF(rate/2);	//????LPF???????
}

//?????
//???:???(???100?)
short MPU_Get_Temperature(void)
{
  u8 buf[2]; 
  short raw;
	float temp;
	
	MPU_Read_Len(MPU_ADDR,MPU_TEMP_OUTH_REG,2,buf); 
  raw=((u16)buf[0]<<8)|buf[1];  
  temp=36.53+((double)raw)/340;  
  
	return temp*100;;
}
//??????(???)
//gx,gy,gz:???x,y,z??????(???)
//???:0,??
//    ??,????
u8 MPU_Get_Gyroscope(short *gx,short *gy,short *gz)
{
  u8 buf[6],res;  
	
	res=MPU_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
	if(res==0)
	{
		*gx=((u16)buf[0]<<8)|buf[1];  
		*gy=((u16)buf[2]<<8)|buf[3];  
		*gz=((u16)buf[4]<<8)|buf[5];
	} 	
  
	return res;;
}

//??????(???)
//gx,gy,gz:???x,y,z??????(???)
//???:0,??
//    ??,????
u8 MPU_Get_Accelerometer(short *ax,short *ay,short *az)
{
  u8 buf[6],res;  
	
	res=MPU_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
	if(res==0)
	{
		*ax=((u16)buf[0]<<8)|buf[1];  
		*ay=((u16)buf[2]<<8)|buf[3];  
		*az=((u16)buf[4]<<8)|buf[5];
	} 	
  
	return res;;
}

//////////////////////////////////////////////////////////////////////////////////////////
static u8				mpu6050_buffer[14];					//iic???????
T_int16_xyz		GYRO_OFFSET,ACC_OFFSET;

u8				GYRO_OFFSET_OK  = 0;
u8				ACC_OFFSET_OK   = 0;  //ПЊЛњКѓаЃбщ

void MPU6050_Read(void)
{
	MPU_Read_Len(MPU_ADDR ,MPU_ACCEL_XOUTH_REG,14, mpu6050_buffer);
}

void mpu6x_dataanl(T_int16_xyz *data_tempacc,T_int16_xyz *data_tempgyr)
{
	vs32 acc_x,acc_y,acc_z,gyr_x,gyr_y,gyr_z;
	
	acc_x=((((int16_t)mpu6050_buffer[0]) << 8) | mpu6050_buffer[1]) - ACC_OFFSET.X;
	acc_y=((((int16_t)mpu6050_buffer[2]) << 8) | mpu6050_buffer[3]) - ACC_OFFSET.Y;
	acc_z=((((int16_t)mpu6050_buffer[4]) << 8) | mpu6050_buffer[5]) - ACC_OFFSET.Z;
	//????ADC
	gyr_x=((((int16_t)mpu6050_buffer[8]) << 8) | mpu6050_buffer[9]) - GYRO_OFFSET.X;
	gyr_y=((((int16_t)mpu6050_buffer[10]) << 8) | mpu6050_buffer[11]) - GYRO_OFFSET.Y;
	gyr_z=((((int16_t)mpu6050_buffer[12]) << 8) | mpu6050_buffer[13]) - GYRO_OFFSET.Z;
	
	acc_x>MPU6050_MAX ? MPU6050_MAX:acc_x;
	acc_x<MPU6050_MIN ? MPU6050_MIN:acc_x;
	acc_y>MPU6050_MAX ? MPU6050_MAX:acc_y;
	acc_y<MPU6050_MIN ? MPU6050_MIN:acc_y;
	acc_z>MPU6050_MAX ? MPU6050_MAX:acc_z;
	acc_z<MPU6050_MIN ? MPU6050_MIN:acc_z;
	gyr_x>MPU6050_MAX ? MPU6050_MAX:gyr_x;
	gyr_x<MPU6050_MIN ? MPU6050_MIN:gyr_x;
	gyr_y>MPU6050_MAX ? MPU6050_MAX:gyr_y;
	gyr_y<MPU6050_MIN ? MPU6050_MIN:gyr_y;
	gyr_z>MPU6050_MAX ? MPU6050_MAX:gyr_z;
	gyr_z<MPU6050_MIN ? MPU6050_MIN:gyr_z;
	
	data_tempacc->X = acc_x;
	data_tempacc->Y = acc_y;
	data_tempacc->Z = acc_z;
	data_tempgyr->X = gyr_x;
	data_tempgyr->Y = gyr_y;
	data_tempgyr->Z = gyr_z;
	
	if(!GYRO_OFFSET_OK)  //????????
	{
		static int32_t	tempgx=0,tempgy=0,tempgz=0;
		static uint8_t cnt_g=0;
 	
		LedA_on;
		
		if(cnt_g==0)
		{
			GYRO_OFFSET.X=0;
			GYRO_OFFSET.Y=0;
			GYRO_OFFSET.Z=0;
			tempgx = 0;
			tempgy = 0;
			tempgz = 0;
			cnt_g = 1;
			return;
		}
		tempgx+= data_tempgyr->X;
		tempgy+= data_tempgyr->Y;
		tempgz+= data_tempgyr->Z;
		if(cnt_g==200)
		{
			GYRO_OFFSET.X=tempgx/cnt_g;
			GYRO_OFFSET.Y=tempgy/cnt_g;
			GYRO_OFFSET.Z=tempgz/cnt_g;
			cnt_g = 0;
			GYRO_OFFSET_OK = 1;
			LedA_off;
			//EE_SAVE_GYRO_OFFSET();//????
			return;
		}
		cnt_g++;
	}
	
	
	if(!ACC_OFFSET_OK)
	{
		static int32_t	tempax=0,tempay=0,tempaz=0;
		static uint8_t cnt_a=0;
    
		LedC_on;
		
		if(cnt_a==0)
		{
			ACC_OFFSET.X = 0;
			ACC_OFFSET.Y = 0;
			ACC_OFFSET.Z = 0;
			tempax = 0;
			tempay = 0;
			tempaz = 0;
			cnt_a = 1;
			return;
		}
		tempax+= data_tempacc->X;
		tempay+= data_tempacc->Y;
		//tempaz+= MPU6050_ACC_LAST.Z;
		if(cnt_a==200)
		{
			ACC_OFFSET.X=tempax/cnt_a;
			ACC_OFFSET.Y=tempay/cnt_a;
			ACC_OFFSET.Z=tempaz/cnt_a;
			cnt_a = 0;
			ACC_OFFSET_OK = 1;
			LedC_off;
			//EE_SAVE_ACC_OFFSET();//????
			return;
		}
		cnt_a++;		
	}
	
}
/////////////////////////////////////////////////////////////////////////////////////////

//IIC???
//addr:???? 
//reg:?????
//len:????
//buf:???
//???:0,??
//    ??,????
u8 MPU_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
	u8 i; 
  
	MPU_IIC_Start(); 
	MPU_IIC_Send_Byte((addr<<1)|0);//??????+???	
	if(MPU_IIC_Wait_Ack())	//????
	{
		MPU_IIC_Stop();		 
		return 1;		
	}
    MPU_IIC_Send_Byte(reg);	//??????
    MPU_IIC_Wait_Ack();		//????
	for(i=0;i<len;i++)
	{
		MPU_IIC_Send_Byte(buf[i]);	//????
		if(MPU_IIC_Wait_Ack())		//??ACK
		{
			MPU_IIC_Stop();	 
			return 1;		 
		}		
	}    
    MPU_IIC_Stop();	 
	return 0;	
} 
//IIC???
//addr:????
//reg:?????????
//len:??????
//buf:?????????
//???:0,??
//    ??,????
u8 MPU_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{ 
 	MPU_IIC_Start(); 
	MPU_IIC_Send_Byte((addr<<1)|0);//??????+???	
	if(MPU_IIC_Wait_Ack())	//????
	{
		MPU_IIC_Stop();		 
		return 1;		
	}
    MPU_IIC_Send_Byte(reg);	//??????
    MPU_IIC_Wait_Ack();		//????
    MPU_IIC_Start();
	MPU_IIC_Send_Byte((addr<<1)|1);//??????+???	
    MPU_IIC_Wait_Ack();		//???? 
	while(len)
	{
		if(len==1)*buf=MPU_IIC_Read_Byte(0);//???,??nACK 
		else *buf=MPU_IIC_Read_Byte(1);		//???,??ACK  
		len--;
		buf++; 
	}    
    MPU_IIC_Stop();	//???????? 
	return 0;	
}
//IIC????? 
//reg:?????
//data:??
//???:0,??
//    ??,????
u8 MPU_Write_Byte(u8 reg,u8 data) 				 
{ 
    MPU_IIC_Start(); 
	MPU_IIC_Send_Byte((MPU_ADDR<<1)|0);//??????+???	
	if(MPU_IIC_Wait_Ack())	//????
	{
		MPU_IIC_Stop();		 
		return 1;		
	}
    MPU_IIC_Send_Byte(reg);	//??????
    MPU_IIC_Wait_Ack();		//???? 
	MPU_IIC_Send_Byte(data);//????
	if(MPU_IIC_Wait_Ack())	//??ACK
	{
		MPU_IIC_Stop();	 
		return 1;		 
	}		 
    MPU_IIC_Stop();	 
	return 0;
}
//IIC????? 
//reg:????? 
//???:?????
u8 MPU_Read_Byte(u8 reg)
{
	u8 res;
	
  MPU_IIC_Start(); 
	MPU_IIC_Send_Byte((MPU_ADDR<<1)|0);//??????+???	
	MPU_IIC_Wait_Ack();		//???? 
  MPU_IIC_Send_Byte(reg);	//??????
  MPU_IIC_Wait_Ack();		//????
  MPU_IIC_Start();
	MPU_IIC_Send_Byte((MPU_ADDR<<1)|1);//??????+???	
  MPU_IIC_Wait_Ack();		//???? 
	res=MPU_IIC_Read_Byte(0);//????,??nACK 
  MPU_IIC_Stop();			//???????? 
	
	return res;		
}



