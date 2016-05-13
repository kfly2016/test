#include "stm32f10x.h"
#include "uart.h"
#include "typeconf.h"


#define BYTE0(dwTemp)       (*(char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))


u8 Send_Status = 0, Send_MotoPwm;
u8 data_to_send[50];

extern T_float_angle 		Att_Angle;	//ATT?????????
extern T_RC_Data 			Rc_D;	
extern vs16 Moto_PWM_1, Moto_PWM_2, Moto_PWM_3, Moto_PWM_4;

void com_recv_data(u8 *data_buf,u8 num)
{
	vs16 rc_value_temp;
	u8 sum = 0;
	u8 i = 0;
	
	for(i=0; i<(num-1); i++)
		sum += *(data_buf+i);  //计算sum值
	
	if(!(sum==*(data_buf+num-1)))		
		return;		             //判断sum值
	
	if(!(*(data_buf)==0xAA && *(data_buf+1)==0xAF))		
		return;		            //判断帧头
	
	
	if(*(data_buf+2)==0x03)        // ??????
	{
		Rc_D.THROTTLE = (vs16)(*(data_buf+4)<<8)|*(data_buf+5);
		Rc_D.YAW = (vs16)(*(data_buf+6)<<8)|*(data_buf+7);
		Rc_D.ROLL = (vs16)(*(data_buf+8)<<8)|*(data_buf+9);
		Rc_D.PITCH = (vs16)(*(data_buf+10)<<8)|*(data_buf+11);
		Rc_D.AUX1 = (vs16)(*(data_buf+12)<<8)|*(data_buf+13);
		Rc_D.AUX2 = (vs16)(*(data_buf+14)<<8)|*(data_buf+15);
		Rc_D.AUX3 = (vs16)(*(data_buf+16)<<8)|*(data_buf+17);
		Rc_D.AUX4 = (vs16)(*(data_buf+18)<<8)|*(data_buf+19);
		Rc_D.AUX5 = (vs16)(*(data_buf+20)<<8)|*(data_buf+21);
		Rc_D.AUX6 = (vs16)(*(data_buf+21)<<8)|*(data_buf+22);
		//ctl_fun(&Rc_D,&Rc_C);
	}
}


//AAFF7CFFFFC043C0808000E720FF8000
//AA AA 01 0D 00 00 00 00 8C A0 BB BB 00 00
void com_send_status(void)
{
	u8 _cnt=0;
	u8 sum = 0;
	u8 i=0;
	vs16 _temp;
	vs32 _temp2 = 0;
	
	data_to_send[_cnt++]=0xAA;
	data_to_send[_cnt++]=0xAA;
	data_to_send[_cnt++]=0x01;
	data_to_send[_cnt++]=0;
	
	_temp = (int)(Att_Angle.rol*100);
	data_to_send[_cnt++]=BYTE1(_temp);
	data_to_send[_cnt++]=BYTE0(_temp);
	
	_temp = (int)(Att_Angle.pit*100);
	data_to_send[_cnt++]=BYTE1(_temp);
	data_to_send[_cnt++]=BYTE0(_temp);
	
	_temp = (int)(Att_Angle.yaw*100);
	data_to_send[_cnt++]=BYTE1(_temp);
	data_to_send[_cnt++]=BYTE0(_temp);
	
	_temp = 0;
	data_to_send[_cnt++]=BYTE1(_temp);
	data_to_send[_cnt++]=BYTE0(_temp);
	
	data_to_send[_cnt++]=BYTE3(_temp2);
	data_to_send[_cnt++]=BYTE2(_temp2);
	data_to_send[_cnt++]=BYTE1(_temp2);
	data_to_send[_cnt++]=BYTE0(_temp2);
		
	//if(Rc_C.ARMED==0)		
		//data_to_send[_cnt++]=0xA0;	
	//else if(Rc_C.ARMED==1)		
	data_to_send[_cnt++]=0xA1;
	
	data_to_send[3] = _cnt-4;
	
	
	for(i=0;i<_cnt;i++)
		sum += data_to_send[i];
	data_to_send[_cnt++]=sum;

	uart_send_buf(data_to_send,_cnt);
}

void com_send_pwm(void)
{
	u8 _cnt=0;
	u8  i=0;
	u8 sum = 0;
	
	data_to_send[_cnt++]=0xAA;
	data_to_send[_cnt++]=0xAA;
	data_to_send[_cnt++]=0x06;
	data_to_send[_cnt++]=0;
	data_to_send[_cnt++]=BYTE1(Moto_PWM_1);
	data_to_send[_cnt++]=BYTE0(Moto_PWM_1);
	data_to_send[_cnt++]=BYTE1(Moto_PWM_2);
	data_to_send[_cnt++]=BYTE0(Moto_PWM_2);
	data_to_send[_cnt++]=BYTE1(Moto_PWM_3);
	data_to_send[_cnt++]=BYTE0(Moto_PWM_3);
	data_to_send[_cnt++]=BYTE1(Moto_PWM_4);
	data_to_send[_cnt++]=BYTE0(Moto_PWM_4);
	data_to_send[_cnt++]=0;
	data_to_send[_cnt++]=0;
	data_to_send[_cnt++]=0;
	data_to_send[_cnt++]=0;
	data_to_send[_cnt++]=0;
	data_to_send[_cnt++]=0;
	data_to_send[_cnt++]=0;
	data_to_send[_cnt++]=0;
	
	data_to_send[3] = _cnt-4;
	
	for(i=0;i<_cnt;i++)
		sum += data_to_send[i];
	
	data_to_send[_cnt++]=sum;
	
	uart_send_buf(data_to_send,_cnt);
}




void com_send_data(void)
{
	if(Send_Status)
	{
		Send_Status = 0;
		com_send_status();
	}
	else if(Send_MotoPwm)
	{
		Send_MotoPwm = 0;
		com_send_pwm();
	}
}

