#include "stm32f10x.h"
#include "stdio.h"
#include "motor.h"

/********************************************************************
º¯Êý¹¦ÄÜ£ºµç»úÖµË¢ÐÂ
Èë¿Ú²ÎÊý£ºint m1_pwm, int m2_pwm, int m3_pwm, int m4_pwm£
·µ    »Ø£ºÎÞ¡£
±¸    ×¢£º
********************************************************************/
void motor_flash(int m1_pwm, int m2_pwm, int m3_pwm, int m4_pwm)
{		
    if(m1_pwm >= MOTO_PWMMAX)	m1_pwm = MOTO_PWMMAX;
    if(m2_pwm >= MOTO_PWMMAX)	m2_pwm = MOTO_PWMMAX;
    if(m3_pwm >= MOTO_PWMMAX)	m3_pwm = MOTO_PWMMAX;
    if(m4_pwm >= MOTO_PWMMAX)	m4_pwm = MOTO_PWMMAX;
    if(m1_pwm <= MOTO_PWMMIN)	m1_pwm = MOTO_PWMMIN;
    if(m2_pwm <= MOTO_PWMMIN)	m2_pwm = MOTO_PWMMIN;
    if(m3_pwm <= MOTO_PWMMIN)	m3_pwm = MOTO_PWMMIN;
    if(m4_pwm <= MOTO_PWMMIN)	m4_pwm = MOTO_PWMMIN;
    
    TIM2->CCR1 = m1_pwm;
    TIM2->CCR2 = m2_pwm;
    TIM2->CCR3 = m3_pwm;
    TIM2->CCR4 = m4_pwm;       
}

/********************************************************************
º¯Êý¹¦ÄÜ£ºµç»ú³õÊ¼»¯
Èë¿Ú²ÎÊý£ºÎÞ¡£
·µ    »Ø£ºÎÞ¡£
±¸    ×¢£º
********************************************************************/
void motor_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    uint16_t PrescalerValue = 0;    //¿ØÖÆµç»úPWMÆµÂÊ
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //´ò¿ªÍâÉèAµÄÊ±ÖÓºÍ¸´ÓÃÊ±ÖÓ
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 ,ENABLE);   //´ò¿ª¶¨Ê±Æ÷2Ê±ÖÓ  
    
    // ÉèÖÃGPIO¹¦ÄÜ¡£
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // ¸´Î»¶¨Ê±Æ÷¡£
    TIM_DeInit(TIM2);
    
    // ÅäÖÃ¼ÆÊ±Æ÷¡£
    PrescalerValue = (uint16_t) (SystemCoreClock / 24000000) - 1;
    
    TIM_TimeBaseStructure.TIM_Period = 999;		            //¼ÆÊýÉÏÏß	
    TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;	//pwmÊ±ÖÓ·ÖÆµ
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;	
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //ÏòÉÏ¼ÆÊý
    TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
    
    // ÅäÖÃTIM2ÎªPWMÊä³öÄ£Ê½
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;    //0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    
    TIM_OC1Init(TIM2,&TIM_OCInitStructure);
    TIM_OC2Init(TIM2,&TIM_OCInitStructure);
    TIM_OC3Init(TIM2,&TIM_OCInitStructure);
    TIM_OC4Init(TIM2,&TIM_OCInitStructure);
    
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    // Æô¶¯¼ÆÊ±Æ÷¡£
    TIM_Cmd(TIM2,ENABLE);
    
		//printf("-> motor enable...\r\n");
}
