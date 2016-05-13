#ifndef _CTL_H_
#define _CTL_H_

#include "stm32f10x.h"
#include "typeconf.h"
#include "motor.h"

typedef struct
{
	float P, I, D;
  float pout, iout, dout;
  float IMAX, OUT;
}PID;

extern PID PID_ROL, PID_PIT,PID_YAW;//PID_ALT,PID_POS;
//extern PID PID_PID_1,PID_PID_2,PID_PID_3,PID_PID_4,PID_PID_5,PID_PID_6,PID_PID_7,PID_PID_8,PID_PID_9,PID_PID_10,PID_PID_11,PID_PID_12;
extern vs16 Moto_PWM_1,Moto_PWM_2,Moto_PWM_3,Moto_PWM_4;//Moto_PWM_5,Moto_PWM_6,Moto_PWM_7,Moto_PWM_8;

void control(T_float_angle *att_in,T_int16_xyz *gyr_in, T_RC_Data *rc_in, u8 armed);

#endif
