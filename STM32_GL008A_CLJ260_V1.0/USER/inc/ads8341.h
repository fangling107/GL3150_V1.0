#ifndef __ADS8341_H
#define __ADS8341_H			    
#include "stm32f10x.h" 
#include "setup.h"  

#define	SPI_AD8341_CS PAout(4)  //ѡ��AD8341
extern u16 pressADvalue;//ADS8341��ADֵ

void SPI_AD8341_Init(void);
u16 SPI_AD8341_Read_ADVal(u8 ch);	//��ȡͨ��ch��adֵ
u16 read_16bit_pressAD(u8 ch, u8 times);	//����10�Σ���ƽ��ֵ

#endif
















