#include "stm32f10x.h"
#include "motor.h"
#include "uart.h"
#include "bt.h"
#include "led.h"
#include "mpu6x.h"
#include "ms56xx.h"

static u8  fac_us=0;//us延时倍乘数
static u16 fac_ms=0;//ms延时倍乘数

/********************************************************************
函数功能：初始化延迟函数
入口参数：SYSCLK:系统时钟
返    回：无。
备    注：SYSTICK的时钟固定为HCLK时钟的1/8
********************************************************************/
static void delay_init(u8 SYSCLK)
{
	SysTick->CTRL&=0xfffffffb;//bit2清空,选择外部时钟  HCLK/8
	fac_us=SYSCLK/8;		    
	fac_ms=(u16)fac_us*1000;
}
								    
/********************************************************************
函数功能：延时nms
入口参数：nms<=0xffffff*8*1000/SYSCLK
返    回：无。
备    注：注意nms的范围
          SysTick->LOAD为24位寄存器,所以,最大延时为:
          SYSCLK单位为Hz,nms单位为ms
          对72M条件下,nms<=1864
********************************************************************/
void delay_ms(u16 nms)
{	 		  	  
  u32 temp;
			   
	SysTick->LOAD=(u32)nms*fac_ms;//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;           //清空计数器
	SysTick->CTRL=0x01 ;          //开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL=0x00;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器  
}   

/********************************************************************
函数功能：延时nus
入口参数：nus为要延时的us数.
返    回：无。
备    注：
********************************************************************/		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; //时间加载	  		 
	SysTick->VAL=0x00;        //清空计数器
	SysTick->CTRL=0x01 ;      //开始倒数 	 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL=0x00;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器	 
}

/********************************************************************
函数功能：设置向量表偏移地址
入口参数：NVIC_VectTab:基址  Offset:偏移量
返    回：无。
备    注：
********************************************************************/
static void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset)	 
{ 
  	//检查参数合法性
	assert_param(IS_NVIC_VECTTAB(NVIC_VectTab));
	assert_param(IS_NVIC_OFFSET(Offset));  	 
	SCB->VTOR = NVIC_VectTab|(Offset & (u32)0x1FFFFF80);//设置NVIC的向量表偏移寄存器
	//用于标识向量表是在CODE区还是在RAM区
}

/********************************************************************
函数功能：rcc时钟初始化函数
入口参数：无
返    回：无。
备    注：把所有时钟寄存器复位 不能在这里执行所有外设复位!否则至少引起串口不工作.
********************************************************************/
static void MYRCC_DeInit(void)
{										  					   
	RCC->APB1RSTR = 0x00000000;//复位结束			 
	RCC->APB2RSTR = 0x00000000; 
	  
  RCC->AHBENR = 0x00000014;  //睡眠模式闪存和SRAM时钟使能.其他关闭.	  
  RCC->APB2ENR = 0x00000000; //外设时钟关闭.			   
  RCC->APB1ENR = 0x00000000;  
	
	RCC->CR |= 0x00000001;     //使能内部高速时钟HSION	 															 
	RCC->CFGR &= 0xF8FF0000;   //复位SW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]					 
	RCC->CR &= 0xFEF6FFFF;     //复位HSEON,CSSON,PLLON
	RCC->CR &= 0xFFFBFFFF;     //复位HSEBYP	   	  
	RCC->CFGR &= 0xFF80FFFF;   //复位PLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE 
	RCC->CIR = 0x00000000;     //关闭所有中断
	//配置向量表				  
#ifdef  VECT_TAB_RAM
	MY_NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else   
	MY_NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}

/********************************************************************
函数功能：系统时钟初始化函数
入口参数：pll:选择的倍频数，从2开始，最大值为16	
返    回：无。
备    注：
********************************************************************/
static void clock_Init(u8 PLL)
{
	u8 temp=0; 
	  
	MYRCC_DeInit();		  //复位并配置向量表
	RCC->CR|=0x00010000;  //外部高速时钟使能HSEON
	while(!(RCC->CR>>17));//等待外部时钟就绪
	RCC->CFGR=0X00000400; //APB1=DIV2;APB2=DIV1;AHB=DIV1;
	PLL-=2;//抵消2个单位
	RCC->CFGR|=PLL<<18;   //设置PLL值 2~16
	RCC->CFGR|=1<<16;	  //PLLSRC ON 
	FLASH->ACR|=0x32;	  //FLASH 2个延时周期

	RCC->CR|=0x01000000;  //PLLON
	while(!(RCC->CR>>25));//等待PLL锁定
	RCC->CFGR|=0x00000002;//PLL作为系统时钟	 
	while(temp!=0x02)     //等待PLL作为系统时钟设置成功
	{   
		temp=RCC->CFGR>>2;
		temp&=0x03;
	}    
}

/********************************************************************
函数功能：TIM3初始化
入口参数：无。
返    回：无。
备    注：1ms中断一次	
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
函数功能：端口初始化
入口参数：无。
返    回：无。
备    注：
********************************************************************/
static void port_init(void)
{}

/********************************************************************
函数功能：初始化设置
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void init_devices(void)
{
   u8 i = 0;
	
	 clock_Init(9);//系统时钟设置
   delay_init(72);
	 bt_init();
	 
	 for(; i < 12; i++)
	  delay_ms(1000);
	 
   port_init();	  //端口初始化
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
   
	 led_init();		
	 tim3_init(500);
	 MPU_Init();
	 uart_init();
//   ms56xx_init();
	 motor_init();
	 
	 tim3_ctl(1);
}
