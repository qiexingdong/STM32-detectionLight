//==========================================================================
//文件名称：rtc.c
//功能概要：STM32L431RC RTC底层驱动程序源文件
//版权所有：苏州大学飞思卡尔嵌入式中心(sumcu.suda.edu.cn)
//==========================================================================
#include "rtc.h"

// ==============================内部函数申明=================================
void Delay_ms(uint16_t u16ms);
uint8_t RTC_DEC2BCD(uint8_t val);
uint8_t RTC_BCD2DEC(uint8_t val);
uint8_t RTC_Init_Mode(void);
// ==========================================================================
// 函数名称：RTC_Init
// 函数参数：无
// 函数返回：0,初始化成功；1,进入初始化失败
// 功能概要：选择LSE时钟,频率为32.768kHz;将7位异步预分频器为128,15位同步预分频器为256;并初始化时钟
// ==========================================================================
uint8_t RTC_Init(void)
{
	RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN_Msk;		    //使能电源接口时钟
	PWR->CR1 |= PWR_CR1_DBP_Msk;     //后备区域访问使能(RTC+SRAM)
	RCC->CSR |= RCC_CSR_LSION_Msk;				//LSI总是使能
	while(!(RCC->CSR&0x02)) ;	    //等待LSI就绪
	RCC->BDCR &= ~(RCC_BDCR_RTCSEL_Msk);             //清零8/9位
	RCC->BDCR |= 1<<9;              //选择LSI时钟
	RCC->BDCR |= RCC_BDCR_RTCEN_Msk;             //使能RTC时钟
	RTC->WPR = 0xCA;                //关闭RTC寄存器写保护
	RTC->WPR = 0x53;
	RTC->CR = 0;
	if(RTC_Init_Mode())              //进入RTC初始化模式失败
	{
		RCC->BDCR = RCC_BDCR_BDRST_Msk;//复位BDCR
		Delay_ms(10);
		RCC->BDCR = 0;			    //结束复位
		return 1;
	}
	RTC->PRER = 0XFF;			    //RTC同步分频系数(0~7FFF),必须先设置同步分频,再设置异步分频
	RTC->PRER |= 0X7F<<16;		    //RTC异步分频系数(1~0X7F)
	RTC->CR &= ~(RTC_CR_FMT_Msk);   //24h制
	RTC->ISR &= ~(RTC_ISR_INIT_Msk);//退出RTC初始化模式
	RTC->WPR = 0XFF;                //使能RTC寄存器写保护
	return 0;
}

// ===========================================================================
// 函数名称：RTC_Set_Date
// 函数参数：year:年份;month:月份;date:天数;week:星期几
// 函数返回：1:设置日期成功;0:设置日期失败
// 功能概要：设置RTC时钟的日期
// ===========================================================================
uint8_t RTC_Set_Date(uint8_t year,uint8_t month,uint8_t date,uint8_t week)
{
	uint32_t temp = 0;
	RTC->WPR = 0xCA;			                   //关闭RTC寄存器写保护
	RTC->WPR = 0x53;
	if(RTC_Init_Mode())                            //进入RTC初始化模式失败
		return 1;
	temp = (((uint32_t)week&0X07)<<13)|((uint32_t)RTC_DEC2BCD(year)<<16)|((uint32_t)RTC_DEC2BCD(month)<<8)|(RTC_DEC2BCD(date));
	RTC->DR = temp;                                //设置RTC的日期寄存器的值
	RTC->ISR &= ~(RTC_ISR_INIT_Msk);			               //退出RTC初始化模式
	return 0;
}

// ===========================================================================
// 函数名称：RTC_Set_Time
// 函数参数：hour:小时;min:分钟;sec:秒钟;
// 函数返回：1:设置时间成功;0:设置时间失败
// 功能概要：设置RTC时钟的时间
// ===========================================================================
uint8_t RTC_Set_Time(uint8_t hour,uint8_t min,uint8_t sec)
{
	uint32_t temp = 0;
	RTC->WPR = 0xCA;                               //关闭RTC寄存器写保护
	RTC->WPR = 0x53;
	if(RTC_Init_Mode())                           //进入RTC初始化模式失败
		return 1;
	temp = (((uint32_t)0&0X01)<<22)|((uint32_t)RTC_DEC2BCD(hour)<<16)|((uint32_t)RTC_DEC2BCD(min)<<8)|(RTC_DEC2BCD(sec));
	RTC->TR = temp;                                //设置RTC的时间寄存器的值
	RTC->ISR &= ~(RTC_ISR_INIT_Msk);			   //退出RTC初始化模式
	return 0;
}

// ===========================================================================
// 函数名称：RTC_Get_Date
// 函数参数：year:年份;month:月份;date:天数;week:星期几
// 函数返回：无
// 功能概要：获取RTC时钟的日期
// ===========================================================================
void RTC_Get_Date(uint8_t *year,uint8_t *month,uint8_t *date,uint8_t *week)
{
	uint32_t temp = 0;
	while(RTC_Wait_Synchro());                      //等待同步到备份寄存器中
	temp = RTC->DR;
	*year = RTC_BCD2DEC((temp>>16)&0XFF);
	*month = RTC_BCD2DEC((temp>>8)&0X1F);
	*date = RTC_BCD2DEC(temp&0X3F);
	*week = (temp>>13)&0X07;
}

// ===========================================================================
// 函数名称：RTC_Get_Time
// 函数参数：hour:小时;min:分钟;sec:秒钟;
// 函数返回：无
// 功能概要：获取RTC时钟的时间
// ===========================================================================
void RTC_Get_Time(uint8_t *hour,uint8_t *min,uint8_t *sec)
{
	uint32_t temp = 0;
	while(RTC_Wait_Synchro());                      //等待同步到备份寄存器中
	temp = RTC->TR;
	*hour = RTC_BCD2DEC((temp>>16)&0X3F);
	*min = RTC_BCD2DEC((temp>>8)&0X7F);
	*sec = RTC_BCD2DEC(temp&0X7F);
	//*ampm = temp>>22;
}

// ==========================================================================
// 函数名称：RTC_Set_Alarm
// 函数参数：SelAlarm：0：闹钟A，1：闹钟B;week:星期数;hour:小时;min:分钟;sec:秒钟
// 函数返回：无
// 功能概要：设置闹钟的时间
// ==========================================================================
void RTC_Set_Alarm(uint8_t SelAlarm,uint8_t week,uint8_t hour,uint8_t min,uint8_t sec)
{
    if(SelAlarm!=0&&SelAlarm!=1) SelAlarm=0;                 //保护，默认选择闹钟A
    RTC->WPR = 0xCA;                                         //关闭RTC寄存器写保护
	RTC->WPR = 0x53;
    if(SelAlarm==0) //闹钟A
    {
		RTC->CR &= ~(RTC_CR_ALRAE_Msk);                          //关闭闹钟A
		while((RTC->ISR & 0x01)==0) ;                            //等待闹钟A修改允许
		RTC->ALRMAR = 0;                                         //清除闹钟A寄存器的设置
		RTC->ALRMAR |= RTC_ALRMAR_WDSEL_Msk;                     //按星期闹铃
		RTC->ALRMAR |= 0<<RTC_ALRMAR_PM_Pos;                      //24小时格式
		RTC->ALRMAR |= (uint32_t)RTC_DEC2BCD(week)<<RTC_ALRMAR_DU_Pos;       //星期设置
		RTC->ALRMAR |= (uint32_t)RTC_DEC2BCD(hour)<<RTC_ALRMAR_HU_Pos;       //小时设置
		RTC->ALRMAR |= (uint32_t)RTC_DEC2BCD(min)<<RTC_ALRMAR_MNU_Pos;	     //分钟设置
		RTC->ALRMAR |= (uint32_t)RTC_DEC2BCD(sec);		         //秒钟设置
		RTC->ALRMASSR = 0;						                 //不使用SUB SEC
	
		RTC->CR |= RTC_CR_ALRAE_Msk;                             //开启闹钟A
		RTC->WPR = 0XFF;		                                 //禁止修改RTC寄存器
		RTC->ISR &= ~(RTC_ISR_ALRAF_Msk);                        //清除RTC闹钟A的标志
		EXTI->PR1 = EXTI_PR1_PIF18_Msk;                          //清除LINE18上的中断标志位 ,写1清0
		EXTI->IMR1 |= EXTI_IMR1_IM18_Msk;                        //开启LINE18上的中断
		EXTI->RTSR1 |= EXTI_RTSR1_RT18_Msk;                      //上升沿触发
	}
	else if(SelAlarm==1) //闹钟B
	{
		RTC->CR &= ~(RTC_CR_ALRBE_Msk);                          //关闭闹钟B
		while((RTC->ISR & 0x02)==0) ;                            //等待闹钟B修改允许
		RTC->ALRMBR = 0;                                         //清除闹钟B寄存器的设置
		RTC->ALRMBR |= RTC_ALRMBR_WDSEL_Msk;                     //按星期闹铃
		RTC->ALRMBR |= 0<<RTC_ALRMBR_PM_Pos;                      //24小时格式
		RTC->ALRMBR |= (uint32_t)RTC_DEC2BCD(week)<<RTC_ALRMBR_DU_Pos;       //星期设置
		RTC->ALRMBR |= (uint32_t)RTC_DEC2BCD(hour)<<RTC_ALRMBR_HU_Pos;       //小时设置
		RTC->ALRMBR |= (uint32_t)RTC_DEC2BCD(min)<<RTC_ALRMBR_MNU_Pos;	     //分钟设置
		RTC->ALRMBR |= (uint32_t)RTC_DEC2BCD(sec);		         //秒钟设置
		RTC->ALRMBSSR = 0;						                 //不使用SUB SEC
	
		RTC->CR |= RTC_CR_ALRBE_Msk;                             //开启闹钟B
		RTC->WPR = 0XFF;		                                 //禁止修改RTC寄存器
		RTC->ISR &= ~(RTC_ISR_ALRBF_Msk);                        //清除RTC闹钟B的标志
		EXTI->PR1 = EXTI_PR1_PIF18_Msk;                          //清除LINE18上的中断标志位 ,写1清0
		EXTI->IMR1 |= EXTI_IMR1_IM18_Msk;                        //开启LINE18上的中断
		EXTI->RTSR1 |= EXTI_RTSR1_RT18_Msk;                      //上升沿触发
	}
}

// ==========================================================================
// 函数名称：RTC_Set_PeriodWakeUp
// 函数参数：rtc_s:自动唤醒的周期，单位为秒
// 函数返回：无
// 功能概要：设置自动唤醒的周期
// ==========================================================================
void RTC_Set_PeriodWakeUp(uint8_t rtc_s)
{
	RTC->WPR = 0xCA;                                          //关闭RTC寄存器保护
	RTC->WPR = 0x53;
	RTC->CR &= ~(RTC_CR_WUTE_Msk);                            //关闭WAKE UP
	while((RTC->ISR&0x04)==0);                                //等待允许修改定时器配置
	RTC->CR &= ~(RTC_CR_WUCKSEL_Msk);                         //清除原来的时钟选择
	RTC->CR |=RTC_CR_WUCKSEL_2;                               //选择 ck_spre 时钟（通常为 1 Hz）
	if(rtc_s>0)
	{
	    RTC->WUTR = rtc_s-1;                                    //唤醒自动重装载寄存器的值
	}
	else 
		RTC->WUTR=0;
	RTC->ISR &= ~(RTC_ISR_WUTF_Msk);		                   //清除RTC WAKE UP的标志
	RTC->CR |= RTC_CR_WUTE_Msk;                                //使能定时器
	RTC->WPR = 0xFF;                                           //禁止修改RTC寄存器
	EXTI->PR1 = EXTI_PR1_PIF20_Msk;                            //清除LINE20上的中断标志位，写1清0
	EXTI->IMR1 |= EXTI_IMR1_IM20_Msk;                          //开启LINE20上的中断
	EXTI->RTSR1 |= EXTI_RTSR1_RT20_Msk;                        //上升沿触发中断
}

// ==========================================================================
// 函数名称：RTC_Alarm_Enable_Int
// 函数参数：SelAlarm：0：闹钟A，1：闹钟B
// 函数返回：无
// 功能概要：使能闹钟中断
// ==========================================================================
void RTC_Alarm_Enable_Int(uint8_t SelAlarm)
{
    if(SelAlarm==0) //闹钟A
    {
		RTC->CR |= RTC_CR_ALRAIE_Msk;                             //开启闹钟A中断
	}
	else if(SelAlarm==1) //闹钟B
	{
		RTC->CR |= RTC_CR_ALRBIE_Msk;                             //开启闹钟B中断
	}
	NVIC_EnableIRQ(RTC_Alarm_IRQn);                          //使能闹钟模块中断
}

// ==========================================================================
// 函数名称：RTC_PeriodWKUP_Enable_Int
// 函数参数：无
// 函数返回：无
// 功能概要：使能自动唤醒中断
// ==========================================================================
void RTC_PeriodWKUP_Enable_Int()
{
	RTC->CR |= RTC_CR_WUTIE_Msk;                             //使能WAKE UP定时器中断
	NVIC_EnableIRQ(RTC_WKUP_IRQn);                           //使能自动唤醒模块中断
}

// ==========================================================================
// 函数名称：RTC_PeriodWKUP_Disable_Int
// 函数参数：无
// 函数返回：无
// 功能概要：禁止自动唤醒中断
// ==========================================================================
void RTC_PeriodWKUP_Disable_Int()
{
	RTC->CR &= ~(RTC_CR_WUTIE_Msk);                           //关闭WAKE UP定时器中断
	NVIC_DisableIRQ(RTC_WKUP_IRQn);                           //禁止自动唤醒中断
}

// ==========================================================================
// 函数名称：RTC_Alarm_Disable_Int
// 函数参数：SelAlarm：0：闹钟A，1：闹钟B;
// 函数返回：无
// 功能概要：禁止闹钟中断
// ==========================================================================
void RTC_Alarm_Disable_Int(uint8_t SelAlarm)
{
	if(SelAlarm==0) //闹钟A
    {
		RTC->CR &= ~(RTC_CR_ALRAIE_Msk);                           //关闭闹钟A中断
	}
	else if(SelAlarm==1) //闹钟B
	{
		RTC->CR &= ~(RTC_CR_ALRBIE_Msk);                           //关闭闹钟B中断
	}
	NVIC_DisableIRQ(RTC_Alarm_IRQn);                          //禁止闹钟中断
}

//=====================================================================
//函数名称：RTC_PeriodWKUP_Get_Int
//函数返回：1：有唤醒中断，0：没有唤醒中断。
//参数说明：无
//功能概要：获取唤醒中断标志。
//=====================================================================
uint8_t RTC_PeriodWKUP_Get_Int()
{
	//获取定时器唤醒中断标志
	if(RTC->ISR&RTC_ISR_WUTF_Msk)
		return 1;
	else
		return 0;
}

//=====================================================================
//函数名称：RTC_PeriodWKUP_Clear
//函数返回：无
//参数说明：无
//功能概要：清除唤醒中断标志。
//=====================================================================
void RTC_PeriodWKUP_Clear()
{
	//清除定时器唤醒中断标志
	RTC->ISR &= ~RTC_ISR_WUTF_Msk;
	EXTI->PR1 |= EXTI_PR1_PIF20_Msk;          //清除LINE20的中断标志
}

//=====================================================================
//函数名称：RTC_Alarm_Get_Int
//函数返回：1：有闹钟中断，0：没有闹钟中断。
//参数说明：SelAlarm：0：闹钟A，1：闹钟B
//功能概要：获取闹钟中断标志。
//=====================================================================
uint8_t RTC_Alarm_Get_Int(uint8_t SelAlarm)
{
	if(SelAlarm==0)  //闹钟A
	{
		if(RTC->ISR&RTC_ISR_ALRAF_Msk)
			return 1;
		else
			return 0;
	}
	else if(SelAlarm==1) //闹钟B
	{
		if(RTC->ISR&RTC_ISR_ALRBF_Msk)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

//=====================================================================
//函数名称：RTC_Alarm_Clear
//函数返回：无
//参数说明：SelAlarm：0：闹钟A，1：闹钟B
//功能概要：清除闹钟中断标志。
//=====================================================================
void RTC_Alarm_Clear(uint8_t SelAlarm)
{

	if(SelAlarm==0)  //闹钟A
	{
		RTC->ISR &= ~RTC_ISR_ALRAF_Msk;       //清闹钟A的中断标志位
		EXTI->PR1 |= EXTI_PR1_PIF18_Msk;      //清除中断线18的中断标志

	}
	else if(SelAlarm==1) //闹钟B
	{
		RTC->ISR &= ~RTC_ISR_ALRBF_Msk;       //清闹钟B的中断标志位
		EXTI->PR1 |= EXTI_PR1_PIF18_Msk;      //清除中断线18的中断标志
	}
}


// ===========================================================================
// 函数名称：RTC_Wait_Synchro
// 函数参数：无
// 函数返回：1:备份失败;0:备份成功。
// 功能概要： 将日历寄存器中的值备份到备用寄存器中
// ===========================================================================
uint8_t RTC_Wait_Synchro(void)
{
	uint32_t retry = 0XFFFFF;
	RTC->WPR = 0xCA;                                 //关闭RTC寄存器写保护
	RTC->WPR = 0x53;
	RTC->ISR &= ~(RTC_ISR_RSF_Msk);		                     //清除RSF位
	while(retry&&((RTC->ISR&(RTC_ISR_RSF_Msk))==0x00))       //等待与备份寄存器同步
		retry--;
    if(retry==0)                                     //同步失败
    	return 1;
	RTC->WPR = 0xFF;			                     //使能RTC寄存器写保护
	return 0;
}
//==============================内部函数定义处=================================
//======================================================================
//函数名称：Delay_us
//函数返回：无
//参数说明：无
//功能概要：延时 - 毫秒级
//======================================================================
void Delay_ms(uint16_t u16ms)
{
	uint32_t u32ctr;
	for(u32ctr = 0; u32ctr < (48000*u16ms); u32ctr++)
	{
		__asm ("NOP");
	}
}

// ===========================================================================
// 函数名称：RTC_DEC2BCD
// 函数参数：十进制数
// 函数返回：十进制数对应的BCD码格式
// 功能概要：将十进制数转化为对应的BCD码格式
// ===========================================================================
uint8_t RTC_DEC2BCD(uint8_t val)
{
	uint8_t bcdhigh = 0;
	while(val>=10)
	{
		bcdhigh++;
		val -= 10;
	}
	return ((uint8_t)(bcdhigh<<4)|val);
}

// ===========================================================================
// 函数名称：RTC_BCD2DEC
// 函数参数：BCD码格式的数
// 函数返回：BCD码对应的十进制数
// 功能概要：将BCD码格式转化为对应的十进制数
// ===========================================================================
uint8_t RTC_BCD2DEC(uint8_t val)
{
	uint8_t temp = 0;
	temp = (val>>4)*10;
	return (temp+(val&0X0F));
}

// ===========================================================================
// 函数名称：RTC_Init_Mode
// 函数参数：无
// 函数返回：0，进入初始化模式成功；1，进入初始化模式失败
// 功能概要：若进入初始化模式失败，则不允许RTC模块初始化
// ===========================================================================
uint8_t RTC_Init_Mode(void)
{
	uint32_t retry = 0XFFFFF;
	if(RTC->ISR&(RTC_ISR_INITF_Msk))
		return 0;
	RTC->ISR |= RTC_ISR_INIT_Msk;	                               //进入RTC初始化模式
	while(retry&&((RTC->ISR&(RTC_ISR_INITF_Msk))==0x00))            //等待进入RTC初始化模式成功
		retry--;
    if(retry==0)
    	return 1;	                                   //同步失败
	else
		return 0; 			                           //同步成功
}

