#ifndef __crc16_H__
#define __crc16_H__
#include "stm32f10x.h"
extern u8 CRCHigh ;	
extern u8	CRCLow  ;
void Cal_CRC(unsigned char *Data, unsigned char Len);
float get_floatmodbus(u8 *p);
void ftoc(float fvalue,u8 *arr);
void cubicSmooth5 ( float in[], float out[], u8 N );
#endif

















