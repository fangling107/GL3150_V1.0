#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "stm32f10x.h"
#include "setup.h"

#define WAIT  	PFin(8)  	//PF8 WAIT
#define INT 	PFin(9)   	//PF9  INT
//#define TP_XY_exchange 
//#define TP_X_swap
//#define TP_Y_swap
#define TP_PRES_DOWN 0x80  		//����������	  
#define TP_CATH_PRES 0x40  		//�а��������� 

#define LCD_X_SIZE 480
#define LCD_Y_SIZE 272

extern	float xfac;					
extern	float yfac;
extern 	short xoff;
extern 	short yoff;
extern u8 touch_flag;	
extern u8 keystatus;
extern u8 sta;//����״̬//b7:����1/�ɿ�0; 
	                     //b6:0,û�а�������;1,�а�������.
//////////////////////////////////////////////////////////////////////////	 
//������EEPROM����ĵ�ַ�����ַ,ռ��12���ֽ�(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)
#define SAVE_ADDR_BASE 00

extern __IO int16_t OS_TimeMS;
extern u16 LCD_X,LCD_Y;		//ʵ������ֵ
extern u16 ADC_value_X,ADC_value_Y;//��������ֵ
u8 TP_Scan(void);
void Print_Decimal(u16 buffer);     
void Printf_Hex(u8 buf);
void Get_XY_Calibration(void);
void XY_Calibration_Function(void);
u8 TP_Get_Adjdata(void);
void Access_TP_coordinate(u8 tp);//������ɨ��
void KeyScan(void) ;
#endif

















