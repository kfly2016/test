#ifndef __TYPECONF_H
#define __TYPECONF_H

#include "stm32f10x.h"

typedef struct
{
	int16_t X;
	int16_t Y;
	int16_t Z;
}T_int16_xyz;


typedef struct
{
	float rol;
	float pit;
	float yaw;
}T_float_angle;

typedef struct
{
				int16_t ROLL;
				int16_t PITCH;
				int16_t THROTTLE;
				int16_t YAW;
				int16_t AUX1;
				int16_t AUX2;
				int16_t AUX3;
				int16_t AUX4;
				int16_t AUX5;
				int16_t AUX6;
}T_RC_Data;


typedef struct
{
				u8	ARMED;
}T_RC_Control;

#endif
