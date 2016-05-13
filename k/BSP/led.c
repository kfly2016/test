#include "stm32f10x.h"
#include "led.h"

/********************************************************************
函数功能：LED初始化
入口参数：无。
返    回：无。
备    注：Led1-->PA11
          Led2-->PA8
          Led3-->PB1
          Led4-->PB3
********************************************************************/
void led_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA, ENABLE);	 
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_3;				 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIOB->ODR|=5<<1;        //PB1,3上拉

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11;	    		
  GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 
	GPIOA->ODR|=9<<0;        //PA1,11上拉
	
  LedA_off;
	LedB_off;
	LedC_off;
	LedD_off;
}
