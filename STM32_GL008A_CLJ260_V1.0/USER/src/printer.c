#include "printer.h"
u8 GU8_PrintFinishIcon;//0 ��æ 1����
//��ʼ����ӡ���˿�.��ʹ��
void Printer_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure; 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOG, ENABLE);	 //ʹ��PA,PC,PG�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	//�˿�����PA.12--CLK
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);				//�����趨������ʼ��GPIOA.12
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);						 //PA.12 �����	

	/***PC6~11--data2~7*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
	GPIO_Init(GPIOC, &GPIO_InitStructure);				//�����趨������ʼ��GPIOC.4~11
	GPIO_ResetBits(GPIOC,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11);//PC.6,7,8,9,10,11 �����	

	/***PG6,7--data0,1***/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	
	GPIO_Init(GPIOG, &GPIO_InitStructure);	  				//������� ��IO���ٶ�Ϊ50MHz
	GPIO_ResetBits(GPIOG,GPIO_Pin_6|GPIO_Pin_7);			//PG.6,7 �����	
	
	/***PG5--STB***/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	    	//STB
	GPIO_Init(GPIOG, &GPIO_InitStructure);	  			//������� ��IO���ٶ�Ϊ50MHz
	GPIO_SetBits(GPIOG,GPIO_Pin_5); 					//PG.5 �����
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;	  	//Busy ���
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 	//���ó����룬Ĭ������	  
	GPIO_Init(GPIOG, &GPIO_InitStructure);			//��ʼ��GPIOG.4
}

void P_WriteData(u8 Data)
{
	P_Data0 = (Data>>0) & 0x01;
	P_Data1 = (Data>>1) & 0x01;
	P_Data2 = (Data>>2) & 0x01;
	P_Data3 = (Data>>3) & 0x01;
	P_Data4 = (Data>>4) & 0x01;
	P_Data5 = (Data>>5) & 0x01;
	P_Data6 = (Data>>6) & 0x01;
	P_Data7 = (Data>>7) & 0x01;	
}
//��ʱ1ms�ӳ���
void delayms(u16 x)
{
	u16 i,j;
	for(i=x;i>0;i--)
		for(j=12000;j>0;j--);	
}

/************************************************************
** ��������: print(uint8 Data)
** ��������: ���ʴ�ӡ��
** �䡡��: ��ӡ���������
*************************************************************/
void pprint(u8 Data)
{  	

    P_STROBE = 1; //STB=0
		P_CLK = 0;
    while (P_busy != 0)
	{
		if(GU8_PrintFinishIcon == 1) 
		{
			GU8_PrintFinishIcon = 0;
			break;
		}
	}; 
	P_WriteData(Data);
	delayms(2);
	P_CLK = 1;
	delayms(2);
	P_CLK = 0;																//74ls273������ ��������
	P_STROBE = 0;                              //STB=1;
	delayms(2);
	P_STROBE = 1;                              //STB=0;
	delayms(2); 
}

/**********************************************************
** ��������: PrintStr(char *Str)
** ��������: ��ӡ��������
** �䡡��: Ҫ��ӡ���ַ�����
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
***********************************************************/
void PrintStr(char *Str)
{
	while(*Str != 0)
	{
		pprint(*Str++);
		delayms(2);
	}
}

/**********************************************************
** ��������: PrintStr(char *Str)
** ��������: ��ӡ��������
** �䡡��: Ҫ��ӡ���ַ�����
** �䡡��: ��
** ȫ�ֱ���: ��
** ����ģ��: ��
***********************************************************/
void PrintStrNnum(char *Str,u8 num)
{
	u8 i=0;
	while((*Str != 0)&&(i<num))
	{
		pprint(*Str++);
		delayms(2);
		i++;
	}
}

 
