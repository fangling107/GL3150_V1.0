/*
********************************************************************************
*  文 件 名：Setup.h
*
*  文件描述：硬件初始化函数声明
*
*  所用芯片：STM32
*
*  创 建 人：023
*
*  版 本 号：
*
*  修改记录
********************************************************************************
*/
#ifndef __SETUP_H__
#define __SETUP_H__
#include "stm32f10x.h"
//IO口操作宏定义
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
//IO口操作,只对单一的IO口!
//确保n的值小于16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入



#define Nacl_On 					(PBout(5)=1)
#define Nacl_OFF 					(PBout(5)=0)
#define DEHS_On 					(PEout(5)=1)
#define DEHS_OFF 					(PEout(5)=0)
#define MOTOR_On 					(PBout(6)=1)
#define MOTOR_OFF 				(PBout(6)=0)
#define Air_cylinder_On 	(PBout(7)=1)
#define Air_cylinder_OFF 	(PBout(7)=0)

#define PORT2REVBUF     AllBuf.Rx2_1
#define PORT2SEDBUF     AllBuf.Tx2_1
#define U2BUFSIZE      	550
#define U2FRAMEDIST    	20


struct Buf_Type   //;所有的缓冲区的分配
{	

    u8 Tx2_1[U2BUFSIZE];  //;串口2的缓冲区
    u8 Rx2_1[U2BUFSIZE];	//;用于保存
    u8 Rx2_2[U2BUFSIZE]; 		
   
};


struct Data_Dust
{
	u8	Data_years[2];
	u8	Data_month;
	u8	Data_day;
	
	u8	Hours;
	u8  Minutes;
	u8	Seconds;
	
	u8	People[2];//操作员代码
	
	u8	num;//样品编号
	
	u8	Press[2];//阻力
	
	u8 Flow;//测试流量//0 :32 1:85
	
	u8	Aerosol;//0 :nacl 1:dehs
 	
	u8 Efficiency_03[2];
	u8 Efficiency_05[2];
	u8 Efficiency_1[2];
	u8 Efficiency_3[2];
	u8 Efficiency_5[2];
	u8 Efficiency_10[2];
};
extern u8 RTC_250MS;
extern u8 RTC_500MS;
extern u8 RTC_12000MS;
extern u8 RTC_13000MS;
extern u8 RTC_2500MS;

extern void InitHardware (void);
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	;
void starttest(void);
void stoptest(void);
u16 Get_Adc_Average(u8 ch,u8 times);
void  Adc_Init(void);
void delay_us(u16 time);
void delay_ms(u16 time);
#endif
