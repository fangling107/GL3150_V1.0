#include "ADS8341.h" 
#include "spi.h"

//////////////////////////////////////////////////////////////////////////////////	 
//ADS8341 ����						  
////////////////////////////////////////////////////////////////////////////////// 


//��ʱus
void delay_1us(u16 m)
{
	u8 i=10;
	for(;m>0;m--)
		while(i--);
}
//��ʼ��SPI FLASH��IO��
void SPI_AD8341_Init(void)
{	
  	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE);	//PORTAʱ��ʹ�� 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  // PA4 ���� 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOA,GPIO_Pin_4);

	SPI1_Init();		   	//��ʼ��SPI

}

u16 SPI_AD8341_Read_ADVal(u8 ch)   
{  
	u8   S0PDR_H = 0,S0PDR_L = 0;
	u16  S0PDR_HL=0; 
	switch(ch)
	{
		case 0:
			ch = 0x94;
			break;
		case 1:
			ch = 0xd4;
			break;
		default:break;
	}
	SPI_AD8341_CS = 1;  
	delay_1us(30);
	SPI_AD8341_CS = 0;			//ʹ������ 
	delay_1us(30);	
	SPI1_ReadWriteByte(ch);    //���Ͷ�ȡ״̬�Ĵ�������
   //------�ж�SPIF�Ƿ���λ��(���͵�1��8λ����)----------
	delay_1us(30);  //??
	S0PDR_H = SPI1_ReadWriteByte(0x00);    //���Ͷ�ȡ״̬�Ĵ�������   
	S0PDR_H=S0PDR_H<<1;    		//��ֻ�к�7λ��Ч
	//------�ж�SPIF�Ƿ���λ��(���͵�2��8λ����)----------
	delay_1us(30); 
	S0PDR_L = SPI1_ReadWriteByte(0x00);	//������������������spiʱ��	 

	if ((S0PDR_L&0X80)!=0)
	{
		S0PDR_H=S0PDR_H|0X01;  //������ֽ����һλ
	}
	S0PDR_L=S0PDR_L<<1; //��ֻ�к�7λ�͵��ֽ���Чλ
	//------�ж�SPIF�Ƿ���λ��(���͵�3��8λ����)---------
	delay_1us(30);
	SPI1_ReadWriteByte(0x00); 

	if ((SPI1->DR & 0X80)!=0)  //��ȡ���λ�����=1��������
	{
		S0PDR_L=S0PDR_L|0X01;  //������ֽ����һλ
	}
	//--ȡ��Ƭѡ���ߵ��ֽ���ϲ�����ֵ��---
	SPI_AD8341_CS = 1;                // ȡ��Ƭѡ
	SPI_AD8341_CS = 1;                // ȡ��Ƭѡ
	S0PDR_HL = (S0PDR_H<<8) + S0PDR_L;
	return(S0PDR_HL);                   //����adֵ			
}

/****************************************************************************
* ����:AdSample()
* ����:10�β�����ȥ�����/��Сֵ��ʣ��8��ȡƽ��ֵ
* ��ڲ���:ch--->ѡ��ͨ����
* ���ڲ���:a--->8����ƽ��ֵ��ԭʼadֵ
****************************************************************************/
//int  AdSample (u8 ch)
u16 read_16bit_pressAD(u8 ch, u8 times)
{  
	u8  i, j; 
	u16  a, rcv_data[50];
	u32   rcv_data_float;

  //10�β���
	for(i=0; i<times; i++) 
	{       
		rcv_data[i] = 0;
    //Stop_EA();
		a = SPI_AD8341_Read_ADVal(ch);
    //Start_EA();
		rcv_data[i] = a; 
//  Dely(100);
	}
  
  //10��������
	for(j=(times-1); j>0; j--)
	{
		for (i=0; i<j; i++) 
		{
			if (rcv_data[i] > rcv_data[i+1])
			{
				a = rcv_data[i+1];
				rcv_data[i+1] = rcv_data[i];
				rcv_data[i] = a;
			}
		}
	}		
  
   //ȥ�����/��Сֵ��ʣ��8����ƽ��ֵ     
	rcv_data_float = 0;
	for(i=1; i<(times-1); i++)
	{
		rcv_data_float += rcv_data[i];
	}
  
	a = rcv_data_float / (times-2);  //ȡƽ��ֵ        
	return a;                          
}


