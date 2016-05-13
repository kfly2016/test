#ifndef _IMU_H_
#define _IMU_H_

#include "stm32f10x.h"
#include "typeconf.h"


#define FILTER_NUM 	  20

#define Acc_G 		0.0011963f				
#define Gyro_G 		0.0610351f				
#define Gyro_Gr		0.0010653f

//extern float 	AngleOffset_Rol,AngleOffset_Pit; 

void prepare_data(T_int16_xyz *acc_in,T_int16_xyz *acc_out);

void IMUupdate(T_int16_xyz *gyr, T_int16_xyz *acc, T_float_angle *angle);


#endif
