//=====================================================================
//文件名称：isr.c（中断处理程序源文件）
//框架提供：SD-ARM（sumcu.suda.edu.cn）
//版本更新：20170801-20191020
//功能描述：提供中断处理程序编程框架
//移植规则：【固定】
//=====================================================================
#include "includes.h"
#include "string.h"
#include "rtc.h"

uint8_t CreateFrame(uint8_t Data,uint8_t * buffer);		//组帧函数声明
void User_SysFun(uint8_t ch);


uint8_t buff1[32];
uint8_t buff2[32];
uint16_t flag = 0;
//======================================================================
//程序名称：UART_User_Handler
//触发条件：UART_User串口收到一个字节触发
//备    注：进入本程序后，可使用uart_get_re_int函数可再进行中断标志判断
// （1-有UART接收中断，0-没有UART接收中断）
//======================================================================
void UART_User_Handler(void)
{
    uint8_t ch,flag;
    DISABLE_INTERRUPTS;      
       ch = uart_re1(UART_User, &flag);  
    if(flag)   
    {
        uart_send1(UART_User,ch);
    }  

    //（4.1）【自行组帧使用（开始)】
    if(CreateFrame(ch,gcRTCBuf))
	{   
        g_RTC_Flag=1;
	}
       //    【自行组帧使用（结束)】
    ENABLE_INTERRUPTS;       
}
//内部函数
void User_SysFun(uint8_t ch)
{
    //（1）收到的一个字节参与组帧
    if(gcRecvLen == 0)  gcRecvLen =useremuart_frame(ch,(uint8_t*)gcRecvBuf);
    //（2）字节进入组帧后，判断gcRecvLen=0？若为0，表示组帧尚未完成，
    //     下次收到一个字节，再继续组帧
    if(gcRecvLen == 0) goto User_SysFun_Exit;
    //（3）至此，gcRecvLen≠0,表示组帧完成，gcRecvLen为帧的长度,校验序列号后（与
    //     根据Flash中倒数一扇区开始的16字节进行比较）
    //     gcRecvBuf[16]进行跳转
    if(strncmp((char *)(gcRecvBuf),(char *)((MCU_SECTOR_NUM-1)*MCU_SECTORSIZE+
       MCU_FLASH_ADDR_START),16) != 0)
    {
        gcRecvLen = 0;         //恢复接收状态
        goto User_SysFun_Exit;
    }
    //（4）至此，不仅收到完整帧，且序号比较也一致， 根据命令字节gcRecvBuf[16]进行跳转
    //若为User串口程序更新命令，则进行程序更新
    switch(gcRecvBuf[16])  //帧标识
    {
        case 0:
            SYSTEM_FUNCTION((uint8_t *)(gcRecvBuf+17));
            gcRecvLen = 0;         //恢复接收状态
        break;
        default:
        break;
    }
User_SysFun_Exit:
    return;
}
//======================================================================
//程序名称：RTC_WKUP_IRQHandler
//函数参数：无
//中断类型：RTC闹钟唤醒中断处理函数
//======================================================================
 void RTC_WKUP_IRQHandler(void)
 {
 	 uint8_t hour,min,sec;
 	 uint8_t  year,month,date,week;
 	 char *p;
	 if(RTC_PeriodWKUP_Get_Int())         //唤醒中断的标志
	 {
	 	 RTC_PeriodWKUP_Clear(); //清除唤醒中断标志
	 	 flag++;
	 	 RTC_Get_Date(&year,&month,&date,&week); //获取RTC记录的日期
		 RTC_Get_Time(&hour,&min,&sec);    //获取RTC记录的时间
		 p=NumToStr("%02d/%02d/%02d %02d:%02d:%02d %d\n",year,month,date,hour,min,sec,week);
		 uart_send_string(UART_User,p);
		 printf("%02d/%02d/%02d %02d:%02d:%02d %d\n",year,month,date,hour,min,sec,week);
	 }


 }
//======================================================================
//程序名称：RTC_Alarm_IRQHandler
//中断类型：RTC闹钟中断处理函数
//======================================================================
void RTC_Alarm_IRQHandler(void)
{

	if(RTC_Alarm_Get_Int(A))            //闹钟A的中断标志位
	{
		RTC_Alarm_Clear(A);       //清闹钟A的中断标志位
		printf("This is ALARM_A!!!\n");
	}
	if(RTC_Alarm_Get_Int(B))            //闹钟A的中断标志位
	{
		RTC_Alarm_Clear(B);       //清闹钟A的中断标志位
		printf("This is ALARM_B!!!\n");
	}
	
}
//内部调用函数
//===========================================================================
//函数名称：CreateFrame
//功能概要：组建数据帧，将待组帧数据加入到数据帧中
//参数说明：Data：待组帧数据
//       buffer:数据帧变量
//函数返回：组帧状态    0-组帧未成功，1-组帧成功
//===========================================================================
uint8_t CreateFrame(uint8_t Data,uint8_t * buffer)
{
    static uint8_t frameLen=0;    //帧的计数器
    uint8_t frameFlag;            //组帧状态

    frameFlag=0;            //组帧状态初始化
    //根据静态变量frameLen组帧
    switch(frameLen)
    {
        case 0:    //第一个数据
        {
            if (Data=='?')    //收到数据是帧头FrameHead
            {
                buffer[0]=Data;
                frameLen++;
                frameFlag=0;        //组帧开始
            }
            break;
        }
        default:    //其他情况
        {
        	
            //如果接收到的不是帧尾
            if(frameLen>=1 && Data!='!')
            {
                buffer[frameLen]=Data;
                frameLen++;
                break;
            }

            //若是末尾数据则组帧成功
            if(Data=='!')
            {
                buffer[frameLen]=Data;
            	frameFlag=1;    //组帧成功
                frameLen=0;     //计数清0，准备重新组帧
                break;
            }
        }
    }
    return frameFlag;                 //返回组帧状态
}





