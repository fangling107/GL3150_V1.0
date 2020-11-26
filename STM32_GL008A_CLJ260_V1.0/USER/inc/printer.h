#ifndef __PRINTER_H
#define __PRINTER_H	 
#include "stm32f10x.h" 
#include "setup.h"
extern u8 GU8_PrintFinishIcon;//0 ��æ 1����

//�˿ڶ���
#define P_busy PGin(4)      // ����ӡ���Ƿ�׼����
#define P_STROBE PGout(5)   //STB
#define P_CLK PAout(12)     //74LS273��������Ч

#define P_Data0 PCout(10)
#define P_Data1 PCout(11)
#define P_Data2 PCout(9)
#define P_Data3 PCout(8)
#define P_Data4 PCout(7)
#define P_Data5 PGout(6)
#define P_Data6 PGout(7)
#define P_Data7 PCout(6)

void Printer_Init(void);//��ʼ����ӡ���˿�		 		
void PrintStr(char *Str);
void pprint(u8 Data);	 
void PrintStrNnum(char *Str,u8 num);
#endif

















