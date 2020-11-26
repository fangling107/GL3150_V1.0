/*
********************************************************************************
*  �� �� ����Serial.c
*
*  �ļ�����������������غ���
*
*  ����оƬ��
*
*  �� �� �ˣ�
*
*  �� �� �ţ�
*
*  �޸ļ�¼��
********************************************************************************
*/
#include "Serial.h"
#include "Setup.h"
#include "crc16.h"
#include "stdio.h"
#include "stdlib.h"
struct USARTDataStream   U2DS;   //;Uart2���������Ŀ��Ʋ���
struct Buf_Type AllBuf;      //;������
extern float Flow;//��õ�����ֵ
extern u8 start_testflag;//�������Ա�־
extern u8 No_Load;//���ر�־
extern u8 Test_over;//������ɱ�־
extern u8 Data_Time_down;

extern u8 CLJ_START;//CLJ����
extern u8 CLJ_close;//CLJ�ر�

extern u8 Data_Back;//���ݷ��سɹ�

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//ѭ������,ֱ���������   
    USART2->DR = (u8) ch;      
	return ch;
}
#endif 
/*----------------------------------------------------------------------------*
 |  ����2��ʼ��������
 *----------------------------------------------------------------------------*/
void SendFirstByte_USART2(void)
{
    U2DS.TBusy = 10;
    USART_SendData(USART2, (u16)*(U2DS.BufAdr + 0));
    U2DS.TIndex = 1;  
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);   
}


/*----------------------------------------------------------------------------*
 |  ����2�жϷ�������
 *----------------------------------------------------------------------------*/
void SendByte_USART2(void)
{    
    if((USART2->SR & USART_FLAG_TXE))
    {
        U2DS.TBusy =10;
        if(U2DS.TIndex >= U2DS.TLong)
        {
            U2DS.TLong = 0;
            U2DS.SedOrd = 0; //;���������֡���
            U2DS.TBusy = 20;
            U2DS.BufAdr = 0;
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);                                                     
            return ;
        }
        USART_SendData(USART2, (u16)*(U2DS.BufAdr + U2DS.TIndex));
        U2DS.TIndex++;
    }
}


/*----------------------------------------------------------------------------*
 |  ����2�жϽ�������
 *----------------------------------------------------------------------------*/
void ReceiveByte_USART2(void)
{
    int temp;
    u8 ch;
    
    if(USART2->SR & USART_FLAG_RXNE)
    {
        U2DS.RecRun = true;
        U2DS.RBet2Byte = 0x00;
        temp = USART_ReceiveData(USART2);
        ch = (u8)temp;
        AllBuf.Rx2_1[U2DS.RIndex] = ch;
        U2DS.RIndex++;
        if(U2DS.RIndex >= U2BUFSIZE)
        {
            U2DS.RIndex = 0; 
        }
    }
}



/*----------------------------------------------------------------------------*
 | ����2��������
 *----------------------------------------------------------------------------*/
void USART2_Rece(void)
{  
    if(U2DS.RBet2Byte >= U2FRAMEDIST) //;����2�������һ���ֽ��Ѿ�20ms,�Խ��յ������ݽ���Ӧ��
    {
        U2DS.RecRun = false;
        U2DS.RBet2Byte = 0x00;

        if(U2DS.RIndex > U2BUFSIZE)
        {
            U2DS.RIndex = 0;
            return;
        }
        
        U2DS.RecLen = U2DS.RIndex;
        U2DS.RIndex = 0;
        U2DS.RecOrd = true;         //;�����յ�����
        U2DS.BufAdr = AllBuf.Rx2_1; //;ָ��������ĵ�ַ
    }
}


/*----------------------------------------------------------------------------*
 |  ����2��������
 *----------------------------------------------------------------------------*/
void USART2_Send(void)
{
    if(U2DS.TLong > U2BUFSIZE)
    {
        return;
    }    
    
    if(U2DS.TBusy == 0)
    {
        if(U2DS.SedOrd == 'E') //;��������
        { 
            U2DS.SedOrd = 'S'; //;���ڷ���
            SendFirstByte_USART2();
        }
    }
}
//�������ã���CLJ260��������
//Addr:moudbus��ַ
//Command:������ 	1:�����������1���ӵ�����(ʵ�ʿ�����Ϊ03)
//								2:��������						  ��ʵ�ʿ�����Ϊ05 00 1e ff 00��
//								3:ֹͣ����							��ʵ�ʿ�����Ϊ05 00 1e 00 00��
void Sendout_uart1_CLJ260(u8 Addr,u8 Command)
{
	u8 m=0;
if(U2DS.SedOrd == 0)//;����1���Ϳ���
{ 	
	AllBuf.Tx2_1[m++]=Addr;
	if(Command==1)
	{	
		AllBuf.Tx2_1[m++]=0x03;	
		AllBuf.Tx2_1[m++]=0x00;
		AllBuf.Tx2_1[m++]=0x08;//��6��ͨ��������
		AllBuf.Tx2_1[m++]=0x00;
		AllBuf.Tx2_1[m++]=0x0C;			
	}	
	else if(Command==2)//��������
	{	
		//CLJ-R260����
		AllBuf.Tx2_1[m++]=0x05;	
		AllBuf.Tx2_1[m++]=0x00;
		AllBuf.Tx2_1[m++]=0x1E;
		AllBuf.Tx2_1[m++]=0xFF;
		AllBuf.Tx2_1[m++]=0x00;	

	}	
	else if(Command==3)	//ֹͣ����
	{	
		AllBuf.Tx2_1[m++]=0x05;	
		AllBuf.Tx2_1[m++]=0x00;
		AllBuf.Tx2_1[m++]=0x1E;		
		AllBuf.Tx2_1[m++]=0x00;
		AllBuf.Tx2_1[m++]=0x00;	

	}
//�ķ���粻�Ǳ�׼��modbusЭ�鲻��Ҫ�õ�CRC
	Cal_CRC(AllBuf.Tx2_1,m);	// ���� CRC16
	AllBuf.Tx2_1[m++]=CRCHigh;//crc
	AllBuf.Tx2_1[m++]=CRCLow;		

	U2DS.TLong=m;    
	U2DS.SedOrd = 'E';  //;����1������������
	U2DS.BufAdr = AllBuf.Tx2_1;//�����ͻ����������ݷ��͵���Ƶ��      
 }
}
//�������ã������������Ʒ��������ȡ����
void Sendout_uart1_Flowmeter(u8 Addr)
{
	u8 m=0;	
if(U2DS.SedOrd == 0)//;����1���Ϳ���
{ 	
	AllBuf.Tx2_1[m++]=Addr;
	AllBuf.Tx2_1[m++]=0x03;	
	AllBuf.Tx2_1[m++]=0x00;
	AllBuf.Tx2_1[m++]=0x02;		
	AllBuf.Tx2_1[m++]=0x00;
	AllBuf.Tx2_1[m++]=0x02;		

	Cal_CRC(AllBuf.Tx2_1,m);	// ���� CRC16
	AllBuf.Tx2_1[m++]=CRCHigh;//crc
	AllBuf.Tx2_1[m++]=CRCLow;		
	 
	U2DS.TLong=m;    
	U2DS.SedOrd = 'E';  //;����1������������
	U2DS.BufAdr = AllBuf.Tx2_1;//�����ͻ����������ݷ��͵���Ƶ��      
}
}
/*----------------------------------------------------------------------------*
 |  ���ں���
 *----------------------------------------------------------------------------*/
void USART_Com(void)
{  
    USART2_Rece();
    USART2_Send();   
}
/*----------------------------------------------------------------------------*
**���ڵ�ͨѶ
 *----------------------------------------------------------------------------*/
void ZD_Port(void)  
{
		u8 i=0;
		u8 len=0;			
		//***********************����2��ͨѶ**********************//
    if(U2DS.RecOrd == true) //;����2���յ�����
    {
        U2DS.RecOrd = false;
				for(i=0;i<U2DS.RecLen;i++)			
					AllBuf.Rx2_2[i]=*(U2DS.BufAdr+i);
				switch(AllBuf.Rx2_2[0])//�жϵ�ַ
				{
					case 0x01://�ϼоߵļ�����							
					case 0x03://�¼оߵļ�����
						if(U2DS.RecLen==29)
						{
							len=27;
 							Cal_CRC(AllBuf.Rx2_2,len);	// ���� CRC16 У��
 							if((CRCHigh == AllBuf.Rx2_2[len])&&(CRCLow == AllBuf.Rx2_2[len+1]))	// CRCУ����ȷ,����������Ч							
								U2DS.RecStatus='R';//���յ�����	
						}	
					else if(U2DS.RecLen==8)
					{
						len=6;
						Cal_CRC(AllBuf.Rx2_2,len);	// ���� CRC16 У��
						if((CRCHigh == AllBuf.Rx2_2[len])&&(CRCLow == AllBuf.Rx2_2[len+1])&&AllBuf.Rx2_2[0]==0x01&&AllBuf.Rx2_2[4]==0xff)//����start
							CLJ_START=1;
						else if((CRCHigh == AllBuf.Rx2_2[len])&&(CRCLow == AllBuf.Rx2_2[len+1])&&AllBuf.Rx2_2[0]==0x01&&AllBuf.Rx2_2[4]==0x00)//����off
							CLJ_close=1;
						else if((CRCHigh == AllBuf.Rx2_2[len])&&(CRCLow == AllBuf.Rx2_2[len+1])&&AllBuf.Rx2_2[0]==0x03&&AllBuf.Rx2_2[4]==0xff)//����start
							CLJ_START=2;
						else if((CRCHigh == AllBuf.Rx2_2[len])&&(CRCLow == AllBuf.Rx2_2[len+1])&&AllBuf.Rx2_2[0]==0x03&&AllBuf.Rx2_2[4]==0x00)//����off
							CLJ_close=2;	
					}	
							break;
					case 0x05:	//������
						len=7;
						Cal_CRC(AllBuf.Rx2_2,len);	// ���� CRC16 У��	
						if((CRCHigh == AllBuf.Rx2_2[len])&&(CRCLow == AllBuf.Rx2_2[len+1]))	// CRCУ����ȷ,����������Ч
							U2DS.RecStatus='R';//���յ�����
							break;				
					default:break;
				}
    }
}

/*----------------------------------------------------------------------------*
**�Դ���2�յ������ݽ��д���
 *----------------------------------------------------------------------------*/
void DataProcess(void)
{	
	u16 flow1=0,flow2=0;
	
	//***********************�Դ���2�յ������ݽ��д���**********************//
		if(U2DS.RecStatus=='R')
		{	
			U2DS.RecStatus=0;
			switch(AllBuf.Rx2_2[0])	// �жϵ�ַ��
			{	
				case 0x01://�ϼо߳������Ӽ�����
						Dust_03_Up=(((u32)AllBuf.Rx2_2[3]<<24)+((u32)AllBuf.Rx2_2[4]<<16)+((u16)AllBuf.Rx2_2[5]<<8)+(AllBuf.Rx2_2[6]));
						Dust_05_Up=(((u32)AllBuf.Rx2_2[7]<<24)+((u32)AllBuf.Rx2_2[8]<<16)+((u16)AllBuf.Rx2_2[9]<<8)+(AllBuf.Rx2_2[10]));
						Dust_1_Up= (((u32)AllBuf.Rx2_2[11]<<24)+((u32)AllBuf.Rx2_2[12]<<16)+((u16)AllBuf.Rx2_2[13]<<8)+(AllBuf.Rx2_2[14]));
						Dust_3_Up= (((u32)AllBuf.Rx2_2[15]<<24)+((u32)AllBuf.Rx2_2[16]<<16)+((u16)AllBuf.Rx2_2[17]<<8)+(AllBuf.Rx2_2[18]));
						Dust_5_Up= (((u32)AllBuf.Rx2_2[19]<<24)+((u32)AllBuf.Rx2_2[20]<<16)+((u16)AllBuf.Rx2_2[21]<<8)+(AllBuf.Rx2_2[22]));
						Dust_10_Up=(((u32)AllBuf.Rx2_2[23]<<24)+((u32)AllBuf.Rx2_2[24]<<16)+((u16)AllBuf.Rx2_2[25]<<8)+(AllBuf.Rx2_2[26]));
						if(start_testflag==1)//���¿�ʼ���԰�ť
						{
							RTC_12000MS=0;						
						}
						break;
				case 0x03://�¼о߳������Ӽ�����				
						Dust_03_Down=(((u32)AllBuf.Rx2_2[3]<<24)+((u32)AllBuf.Rx2_2[4]<<16)+((u16)AllBuf.Rx2_2[5]<<8)+(AllBuf.Rx2_2[6]));
						Dust_05_Down=(((u32)AllBuf.Rx2_2[7]<<24)+((u32)AllBuf.Rx2_2[8]<<16)+((u16)AllBuf.Rx2_2[9]<<8)+(AllBuf.Rx2_2[10]));
						Dust_1_Down= (((u32)AllBuf.Rx2_2[11]<<24)+((u32)AllBuf.Rx2_2[12]<<16)+((u16)AllBuf.Rx2_2[13]<<8)+(AllBuf.Rx2_2[14]));
						Dust_3_Down= (((u32)AllBuf.Rx2_2[15]<<24)+((u32)AllBuf.Rx2_2[16]<<16)+((u16)AllBuf.Rx2_2[17]<<8)+(AllBuf.Rx2_2[18]));
// 						Dust_5_Down= (((u32)AllBuf.Rx2_2[19]<<24)+((u32)AllBuf.Rx2_2[20]<<16)+((u16)AllBuf.Rx2_2[21]<<8)+(AllBuf.Rx2_2[22]));
// 						Dust_10_Down=(((u32)AllBuf.Rx2_2[23]<<24)+((u32)AllBuf.Rx2_2[24]<<16)+((u16)AllBuf.Rx2_2[25]<<8)+(AllBuf.Rx2_2[26]));
						Dust_5_Down=0;
						Dust_10_Down=0;
						Data_Time_down++;//�¼о߶�������
						if(start_testflag==1)//���¿�ʼ���԰�ť
						{
							RTC_13000MS=0;						
						}
						if(Data_Time_down==6)//ȡ����ɣ����������Ч��
						{
							Data_Time_down=0;
							Efficiency_03=(1-((float)(Dust_03_Down)/(Dust_03_Up)))*100;
							Efficiency_05=(1-((float)(Dust_05_Down)/(Dust_05_Up)))*100;
							Efficiency_1=(1-((float)(Dust_1_Down)/(Dust_1_Up)))*100;
							Efficiency_3=(1-((float)(Dust_3_Down)/(Dust_3_Up)))*100;
							Efficiency_5=(1-((float)(Dust_5_Down)/(Dust_5_Up)))*100;
							Efficiency_10=(1-((float)(Dust_10_Down)/(Dust_10_Up)))*100;
							Test_over=1;
						}						
						break;
				case 0x05://������
						flow1=AllBuf.Rx2_2[3]*256+AllBuf.Rx2_2[4];
						flow2=AllBuf.Rx2_2[5]*256+AllBuf.Rx2_2[6];
						Flow=(float)(flow1*65535+flow2)/1000;
						
					break;				
			}
				
		}
}

