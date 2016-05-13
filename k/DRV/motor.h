#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"

#define MOTO_PWMMAX  	999
#define MOTO_PWMMIN  	0


void motor_flash(int ,int ,int ,int);
void motor_init(void);

#endif
