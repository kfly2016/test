#include "stm32f10x.h"
#include "motor.h"
#include "uart.h"
#include "bt.h"
#include "led.h"
#include "mpu6x.h"
#include "ms56xx.h"

static u8  fac_us=0;//us��ʱ������
static u16 fac_ms=0;//ms��ʱ������

/********************************************************************
�������ܣ���ʼ���ӳٺ���
��ڲ�����SYSCLK:ϵͳʱ��
��    �أ��ޡ�
��    ע��SYSTICK��ʱ�ӹ̶�ΪHCLKʱ�ӵ�1/8
********************************************************************/
static void delay_init(u8 SYSCLK)
{
	SysTick->CTRL&=0xfffffffb;//bit2���,ѡ���ⲿʱ��  HCLK/8
	fac_us=SYSCLK/8;		    
	fac_ms=(u16)fac_us*1000;
}
								    
/********************************************************************
�������ܣ���ʱnms
��ڲ�����nms<=0xffffff*8*1000/SYSCLK
��    �أ��ޡ�
��    ע��ע��nms�ķ�Χ
          SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:
          SYSCLK��λΪHz,nms��λΪms
          ��72M������,nms<=1864
********************************************************************/
void delay_ms(u16 nms)
{	 		  	  
  u32 temp;
			   
	SysTick->LOAD=(u32)nms*fac_ms;//ʱ�����(SysTick->LOADΪ24bit)
	SysTick->VAL =0x00;           //��ռ�����
	SysTick->CTRL=0x01 ;          //��ʼ����  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//�ȴ�ʱ�䵽��   
	SysTick->CTRL=0x00;       //�رռ�����
	SysTick->VAL =0X00;       //��ռ�����  
}   

/********************************************************************
�������ܣ���ʱnus
��ڲ�����nusΪҪ��ʱ��us��.
��    �أ��ޡ�
��    ע��
********************************************************************/		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; //ʱ�����	  		 
	SysTick->VAL=0x00;        //��ռ�����
	SysTick->CTRL=0x01 ;      //��ʼ���� 	 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//�ȴ�ʱ�䵽��   
	SysTick->CTRL=0x00;       //�رռ�����
	SysTick->VAL =0X00;       //��ռ�����	 
}

/********************************************************************
�������ܣ�����������ƫ�Ƶ�ַ
��ڲ�����NVIC_VectTab:��ַ  Offset:ƫ����
��    �أ��ޡ�
��    ע��
********************************************************************/
static void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset)	 
{ 
  	//�������Ϸ���
	assert_param(IS_NVIC_VECTTAB(NVIC_VectTab));
	assert_param(IS_NVIC_OFFSET(Offset));  	 
	SCB->VTOR = NVIC_VectTab|(Offset & (u32)0x1FFFFF80);//����NVIC��������ƫ�ƼĴ���
	//���ڱ�ʶ����������CODE��������RAM��
}

/********************************************************************
�������ܣ�rccʱ�ӳ�ʼ������
��ڲ�������
��    �أ��ޡ�
��    ע��������ʱ�ӼĴ�����λ ����������ִ���������踴λ!�����������𴮿ڲ�����.
********************************************************************/
static void MYRCC_DeInit(void)
{										  					   
	RCC->APB1RSTR = 0x00000000;//��λ����			 
	RCC->APB2RSTR = 0x00000000; 
	  
  RCC->AHBENR = 0x00000014;  //˯��ģʽ�����SRAMʱ��ʹ��.�����ر�.	  
  RCC->APB2ENR = 0x00000000; //����ʱ�ӹر�.			   
  RCC->APB1ENR = 0x00000000;  
	
	RCC->CR |= 0x00000001;     //ʹ���ڲ�����ʱ��HSION	 															 
	RCC->CFGR &= 0xF8FF0000;   //��λSW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]					 
	RCC->CR &= 0xFEF6FFFF;     //��λHSEON,CSSON,PLLON
	RCC->CR &= 0xFFFBFFFF;     //��λHSEBYP	   	  
	RCC->CFGR &= 0xFF80FFFF;   //��λPLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE 
	RCC->CIR = 0x00000000;     //�ر������ж�
	//����������				  
#ifdef  VECT_TAB_RAM
	MY_NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else   
	MY_NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}

/********************************************************************
�������ܣ�ϵͳʱ�ӳ�ʼ������
��ڲ�����pll:ѡ��ı�Ƶ������2��ʼ�����ֵΪ16	
��    �أ��ޡ�
��    ע��
********************************************************************/
static void clock_Init(u8 PLL)
{
	u8 temp=0; 
	  
	MYRCC_DeInit();		  //��λ������������
	RCC->CR|=0x00010000;  //�ⲿ����ʱ��ʹ��HSEON
	while(!(RCC->CR>>17));//�ȴ��ⲿʱ�Ӿ���
	RCC->CFGR=0X00000400; //APB1=DIV2;APB2=DIV1;AHB=DIV1;
	PLL-=2;//����2����λ
	RCC->CFGR|=PLL<<18;   //����PLLֵ 2~16
	RCC->CFGR|=1<<16;	  //PLLSRC ON 
	FLASH->ACR|=0x32;	  //FLASH 2����ʱ����

	RCC->CR|=0x01000000;  //PLLON
	while(!(RCC->CR>>25));//�ȴ�PLL����
	RCC->CFGR|=0x00000002;//PLL��Ϊϵͳʱ��	 
	while(temp!=0x02)     //�ȴ�PLL��Ϊϵͳʱ�����óɹ�
	{   
		temp=RCC->CFGR>>2;
		temp&=0x03;
	}    
}

/********************************************************************
�������ܣ�TIM3��ʼ��
��ڲ������ޡ�
��    �أ��ޡ�
��    ע��1ms�ж�һ��	
********************************************************************/
static void tim3_init(u16 period_num)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	TIM_DeInit(TIM3);

	TIM_TimeBaseStructure.TIM_Period = period_num;//???
	TIM_TimeBaseStructure.TIM_Prescaler=72-1;//??? 
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; //or TIM_CKD_DIV2 or TIM_CKD_DIV4
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM3,TIM_FLAG_Update);
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	
	//?????NVIC??
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3??
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //?????0?
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //????3?
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ?????
	NVIC_Init(&NVIC_InitStructure);  //???NVIC???
	
}

void tim3_ctl(u8 sta)
{
	if(sta==0)
		TIM_Cmd(TIM3,DISABLE);
	if(sta==1)
		TIM_Cmd(TIM3,ENABLE);
}


/********************************************************************
�������ܣ��˿ڳ�ʼ��
��ڲ������ޡ�
��    �أ��ޡ�
��    ע��
********************************************************************/
static void port_init(void)
{}

/********************************************************************
�������ܣ���ʼ������
��ڲ������ޡ�
��    �أ��ޡ�
��    ע���ޡ�
********************************************************************/
void init_devices(void)
{
   u8 i = 0;
	
	 clock_Init(9);//ϵͳʱ������
   delay_init(72);
	 bt_init();
	 
	 for(; i < 12; i++)
	  delay_ms(1000);
	 
   port_init();	  //�˿ڳ�ʼ��
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 	//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
   
	 led_init();		
	 tim3_init(500);
	 MPU_Init();
	 uart_init();
//   ms56xx_init();
	 motor_init();
	 
	 tim3_ctl(1);
}
