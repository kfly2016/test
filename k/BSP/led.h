#ifndef _Led_H_
#define _Led_H_

#include "stm32f10x.h"


#define LedA_on    GPIO_SetBits(GPIOA, GPIO_Pin_11)
#define LedA_off   GPIO_ResetBits(GPIOA, GPIO_Pin_11)


#define LedB_on    GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define LedB_off   GPIO_ResetBits(GPIOA, GPIO_Pin_8)


#define LedC_on    GPIO_SetBits(GPIOB, GPIO_Pin_1)
#define LedC_off   GPIO_ResetBits(GPIOB, GPIO_Pin_1)


#define LedD_on    GPIO_SetBits(GPIOB, GPIO_Pin_3)
#define LedD_off   GPIO_ResetBits(GPIOB, GPIO_Pin_3)

#define LEDA_troggle GPIO_WriteBit(GPIOA,GPIO_Pin_11, !GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_11))
#define LEDB_troggle GPIO_WriteBit(GPIOA,GPIO_Pin_8, !GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_8))
#define LEDC_troggle GPIO_WriteBit(GPIOB,GPIO_Pin_1, !GPIO_ReadOutputDataBit(GPIOB,GPIO_Pin_1))



void led_init(void);  

#endif
