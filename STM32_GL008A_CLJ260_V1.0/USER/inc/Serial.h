/*
********************************************************************************
*  文 件 名：Serial.h
*
*  文件描述：串口相关头文件
*
*  所用芯片：STM32
*
*  创 建 人：023
*
*  版 本 号：
*
*  修改记录：2012-10-17 14:29:17
********************************************************************************
*/
#ifndef __SERIAL_H__
#define __SERIAL_H__
#include "stm32f10x.h"

struct USARTDataStream  //;串口数据流的控制
{    
    u16 RIndex;    		//;接收序列号
    u16 TIndex;   		//;发送序列号
    u16 TLong;     		//;发送长度
    u16 TBusy;     		//;发送忙碌
    u8  RecRun;    		//;是否在接受
    u8  RBet2Byte; 		//;接收两个字节的时间间隔
    u8  RecOrd;    		//;串口收到数据帧
		u8  RecStatus;    //;串口收到数据帧标志
    u16 RecLen;    		//;接收长度
    u8  SedOrd;    		//;串口在发送帧		
    u8  *BufAdr;   	//;发送与接收缓存的指针
};

//颗粒数
extern u32	Dust_03_Up;
extern u32	Dust_05_Up;
extern u32	Dust_1_Up;
extern u32	Dust_3_Up;
extern u32	Dust_5_Up;
extern u32	Dust_10_Up;
//下游颗粒数
extern u32	Dust_03_Down;
extern u32	Dust_05_Down;
extern u32	Dust_1_Down;
extern u32	Dust_3_Down;
extern u32	Dust_5_Down;
extern u32	Dust_10_Down;
//过滤效率
extern float Efficiency_03;
extern float Efficiency_05;
extern float Efficiency_1;
extern float Efficiency_3;
extern float Efficiency_5;
extern float Efficiency_10;
// //上游颗粒数零点
// extern u32	Dust_03_Up_Zero;
// extern u32	Dust_05_Up_Zero;
// extern u32	Dust_1_Up_Zero;
// extern u32	Dust_3_Up_Zero;
// extern u32	Dust_5_Up_Zero;
// extern u32	Dust_10_Up_Zero;
//下游颗粒数零点
// extern u32	Dust_03_Down_Zero;
// extern u32	Dust_05_Down_Zero;
// extern u32	Dust_1_Down_Zero;
// extern u32	Dust_3_Down_Zero;
// extern u32	Dust_5_Down_Zero;
// extern u32	Dust_10_Down_Zero;

#define false   0x00
#define true    0x01

extern struct USARTDataStream  U2DS;
extern void SendFirstByte_USART2(void);
extern void SendByte_USART2(void);
extern void ReceiveByte_USART2(void);

extern void USART_Com(void);
extern void ZD_Port(void);
extern void DataProcess(void);

void Sendout_uart1_CLJ260(u8 Addr,u8 Command);
void Sendout_uart1_Flowmeter(u8 Addr);
#endif
