/*
********************************************************************************
*  �� �� ����Serial.h
*
*  �ļ��������������ͷ�ļ�
*
*  ����оƬ��STM32
*
*  �� �� �ˣ�023
*
*  �� �� �ţ�
*
*  �޸ļ�¼��2012-10-17 14:29:17
********************************************************************************
*/
#ifndef __SERIAL_H__
#define __SERIAL_H__
#include "stm32f10x.h"

struct USARTDataStream  //;�����������Ŀ���
{    
    u16 RIndex;    		//;�������к�
    u16 TIndex;   		//;�������к�
    u16 TLong;     		//;���ͳ���
    u16 TBusy;     		//;����æµ
    u8  RecRun;    		//;�Ƿ��ڽ���
    u8  RBet2Byte; 		//;���������ֽڵ�ʱ����
    u8  RecOrd;    		//;�����յ�����֡
		u8  RecStatus;    //;�����յ�����֡��־
    u16 RecLen;    		//;���ճ���
    u8  SedOrd;    		//;�����ڷ���֡		
    u8  *BufAdr;   	//;��������ջ����ָ��
};

//������
extern u32	Dust_03_Up;
extern u32	Dust_05_Up;
extern u32	Dust_1_Up;
extern u32	Dust_3_Up;
extern u32	Dust_5_Up;
extern u32	Dust_10_Up;
//���ο�����
extern u32	Dust_03_Down;
extern u32	Dust_05_Down;
extern u32	Dust_1_Down;
extern u32	Dust_3_Down;
extern u32	Dust_5_Down;
extern u32	Dust_10_Down;
//����Ч��
extern float Efficiency_03;
extern float Efficiency_05;
extern float Efficiency_1;
extern float Efficiency_3;
extern float Efficiency_5;
extern float Efficiency_10;
// //���ο��������
// extern u32	Dust_03_Up_Zero;
// extern u32	Dust_05_Up_Zero;
// extern u32	Dust_1_Up_Zero;
// extern u32	Dust_3_Up_Zero;
// extern u32	Dust_5_Up_Zero;
// extern u32	Dust_10_Up_Zero;
//���ο��������
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
