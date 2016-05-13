/***************************************************************************************
 
***************************************************************************************/


#include "stm32f10x.h"
#include "bsp.h"
#include "motor.h"
#include "mpu6x.h"
#include "imu.h"
#include "ms56xx.h"
#include "ctl.h"

#include "led.h"

/*******************************************************************************
* 函数名  : main
* 描述    : 主函数，用户程序从main函数开始运行
* 输入    : 无
* 输出    : 无
* 返回值  : int:返回值为一个16位整形数
* 说明    : 无
*******************************************************************************/
u8 FLAG_ATT=0;
T_int16_xyz 		Acc,Gyr;	//???????????
T_int16_xyz			Acc_AVG;
T_float_angle 		Att_Angle;	//ATT?????????
T_RC_Data 			Rc_D;		//??????
T_RC_Control		Rc_C;		//??????

extern u8 GYRO_OFFSET_OK , ACC_OFFSET_OK;
extern u8 Send_Status ,Send_MotoPwm;

int main(void)
{
	int tmp = 0;
	float temp = 0.0;
	
	static u8 rc_cnt=0;
	static u8 att_cnt=0;
	
	static T_int16_xyz mpu6050_dataacc1,
		                 mpu6050_dataacc2,
		                 mpu6050_datagyr1,
		                 mpu6050_datagyr2;
	//short temp;					//??
	
	static u16 senser_cnt=0,
		        status_cnt=0,
		        dt_rc_cnt=0,
		        dt_moto_cnt=0;
	 
  u8 flag = 1;	
	
	init_devices();	
	init_pid();
	
	Rc_D.THROTTLE = 1200;
	Rc_D.ROLL = 1500;
	Rc_D.PITCH = 1500;
	Rc_D.YAW = 1500;
	Rc_C.ARMED = 1;
	
	/*while(1)
	{
		//ms56xx_startConversion(MS561101BA_D2 + MS5611Temp_OSR);
		
    if(flag)
			tmp ++ ;
		else 
			tmp--;
	
 		
		if(tmp > 995) {flag = 0; LedC_on;}
		else if(tmp < 1) {flag = 1; LedC_off; }
		
		motor_flash(tmp, tmp, tmp, tmp);
		delay_ms(300);
		//delay_ms(1000);
		
		//temp = ms56xx_getTemperature();
	   //temp=MPU_Get_Temperature();	//?????
		//test_send_data(10,20,30,40,50,60);
		//printf("this is test \r\n");
	}
	*/
	
	while (1)
	{
		
    if(FLAG_ATT)
		{	
		    FLAG_ATT = 0;
			  
			  att_cnt++;  
			  rc_cnt++;
			
			  if(rc_cnt==20)
			  {
				  rc_cnt = 0;
				//#ifdef CONTROL_USE_RC
				//Rc_GetValue(&Rc_D);
				//#endif
				  ctl_fun(&Rc_D,&Rc_C);
			  }
			
			
			  if(att_cnt==1)
		    {
				  mpu6x_dataanl(&mpu6050_dataacc1,&mpu6050_datagyr1);
		    }
		    else
		    {
				  att_cnt = 0;
				  mpu6x_dataanl(&mpu6050_dataacc2,&mpu6050_datagyr2);
					
				  Acc.X = (mpu6050_dataacc1.X+mpu6050_dataacc2.X)/2;
				  Acc.Y = (mpu6050_dataacc1.Y+mpu6050_dataacc2.Y)/2;
				  Acc.Z = (mpu6050_dataacc1.Z+mpu6050_dataacc2.Z)/2;
				  Gyr.X = (mpu6050_datagyr1.X+mpu6050_datagyr2.X)/2;
				  Gyr.Y = (mpu6050_datagyr1.Y+mpu6050_datagyr2.Y)/2;
				  Gyr.Z = (mpu6050_datagyr1.Z+mpu6050_datagyr2.Z)/2;
					
				  prepare_data(&Acc,&Acc_AVG);
				  IMUupdate(&Gyr,&Acc_AVG,&Att_Angle);
					
					//if(GYRO_OFFSET_OK && ACC_OFFSET_OK)
					//{
					control(&Att_Angle, &Gyr, &Rc_D, Rc_C.ARMED);
					//}
         
					senser_cnt++; 
				  status_cnt++;
				  dt_rc_cnt++;
				  dt_moto_cnt++;
				/*	
					if(flag == 1)
					Rc_D.THROTTLE++;
          else
          Rc_D.THROTTLE--;						
					
					if(Rc_D.THROTTLE == 1900) 
					{
						flag = 0;
					}	
					if(Rc_D.THROTTLE == 1250)
					  flag = 1;
			*/	
					if(status_cnt==50)
				  {
					  //Att_Angle.pit++;
					 //if(Att_Angle.pit>360)
						 // Att_Angle.pit = 0; 
						
						status_cnt = 0;
					  //Send_Status = 1; //??roll pitch yaw????
				  }
					
					if(dt_moto_cnt==100)
				  {
					  dt_moto_cnt=0;
					  //Send_MotoPwm = 1; //??PWM??
				  }
					
	      }
	   }
	}
}


