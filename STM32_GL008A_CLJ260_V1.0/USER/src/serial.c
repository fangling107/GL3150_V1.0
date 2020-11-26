/*
********************************************************************************
*  文 件 名：Serial.c
*
*  文件描述：串口驱动相关函数
*
*  所用芯片：
*
*  创 建 人：
*
*  版 本 号：
*
*  修改记录：
********************************************************************************
*/
#include "Serial.h"
#include "Setup.h"
#include "crc16.h"
#include "stdio.h"
#include "stdlib.h"
struct USARTDataStream   U2DS;   //;Uart2的数据流的控制参数
struct Buf_Type AllBuf;      //;缓冲区
extern float Flow;//测得的流量值
extern u8 start_testflag;//启动测试标志
extern u8 No_Load;//空载标志
extern u8 Test_over;//测试完成标志
extern u8 Data_Time_down;

extern u8 CLJ_START;//CLJ启动
extern u8 CLJ_close;//CLJ关闭

extern u8 Data_Back;//数据返回成功

//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
    USART2->DR = (u8) ch;      
	return ch;
}
#endif 
/*----------------------------------------------------------------------------*
 |  串口2开始发送数据
 *----------------------------------------------------------------------------*/
void SendFirstByte_USART2(void)
{
    U2DS.TBusy = 10;
    USART_SendData(USART2, (u16)*(U2DS.BufAdr + 0));
    U2DS.TIndex = 1;  
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);   
}


/*----------------------------------------------------------------------------*
 |  串口2中断发送数据
 *----------------------------------------------------------------------------*/
void SendByte_USART2(void)
{    
    if((USART2->SR & USART_FLAG_TXE))
    {
        U2DS.TBusy =10;
        if(U2DS.TIndex >= U2DS.TLong)
        {
            U2DS.TLong = 0;
            U2DS.SedOrd = 0; //;串口命令发送帧完成
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
 |  串口2中断接收数据
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
 | 串口2接收命令
 *----------------------------------------------------------------------------*/
void USART2_Rece(void)
{  
    if(U2DS.RBet2Byte >= U2FRAMEDIST) //;串口2接收最后一个字节已经20ms,对接收到的数据进行应答
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
        U2DS.RecOrd = true;         //;串口收到命令
        U2DS.BufAdr = AllBuf.Rx2_1; //;指向接收区的地址
    }
}


/*----------------------------------------------------------------------------*
 |  串口2发送命令
 *----------------------------------------------------------------------------*/
void USART2_Send(void)
{
    if(U2DS.TLong > U2BUFSIZE)
    {
        return;
    }    
    
    if(U2DS.TBusy == 0)
    {
        if(U2DS.SedOrd == 'E') //;发送命令
        { 
            U2DS.SedOrd = 'S'; //;正在发送
            SendFirstByte_USART2();
        }
    }
}
//函数作用：向CLJ260发送命令
//Addr:moudbus地址
//Command:控制码 	1:读计数器最近1分钟的数据(实际控制码为03)
//								2:启动测量						  （实际控制码为05 00 1e ff 00）
//								3:停止测量							（实际控制码为05 00 1e 00 00）
void Sendout_uart1_CLJ260(u8 Addr,u8 Command)
{
	u8 m=0;
if(U2DS.SedOrd == 0)//;串口1发送空闲
{ 	
	AllBuf.Tx2_1[m++]=Addr;
	if(Command==1)
	{	
		AllBuf.Tx2_1[m++]=0x03;	
		AllBuf.Tx2_1[m++]=0x00;
		AllBuf.Tx2_1[m++]=0x08;//读6个通道的数据
		AllBuf.Tx2_1[m++]=0x00;
		AllBuf.Tx2_1[m++]=0x0C;			
	}	
	else if(Command==2)//启动测量
	{	
		//CLJ-R260代码
		AllBuf.Tx2_1[m++]=0x05;	
		AllBuf.Tx2_1[m++]=0x00;
		AllBuf.Tx2_1[m++]=0x1E;
		AllBuf.Tx2_1[m++]=0xFF;
		AllBuf.Tx2_1[m++]=0x00;	

	}	
	else if(Command==3)	//停止测量
	{	
		AllBuf.Tx2_1[m++]=0x05;	
		AllBuf.Tx2_1[m++]=0x00;
		AllBuf.Tx2_1[m++]=0x1E;		
		AllBuf.Tx2_1[m++]=0x00;
		AllBuf.Tx2_1[m++]=0x00;	

	}
//四方光电不是标准的modbus协议不需要用到CRC
	Cal_CRC(AllBuf.Tx2_1,m);	// 计算 CRC16
	AllBuf.Tx2_1[m++]=CRCHigh;//crc
	AllBuf.Tx2_1[m++]=CRCLow;		

	U2DS.TLong=m;    
	U2DS.SedOrd = 'E';  //;串口1发送命令允许
	U2DS.BufAdr = AllBuf.Tx2_1;//将发送缓冲区的数据发送到变频器      
 }
}
//函数作用：向气体流量计发送命令，读取流量
void Sendout_uart1_Flowmeter(u8 Addr)
{
	u8 m=0;	
if(U2DS.SedOrd == 0)//;串口1发送空闲
{ 	
	AllBuf.Tx2_1[m++]=Addr;
	AllBuf.Tx2_1[m++]=0x03;	
	AllBuf.Tx2_1[m++]=0x00;
	AllBuf.Tx2_1[m++]=0x02;		
	AllBuf.Tx2_1[m++]=0x00;
	AllBuf.Tx2_1[m++]=0x02;		

	Cal_CRC(AllBuf.Tx2_1,m);	// 计算 CRC16
	AllBuf.Tx2_1[m++]=CRCHigh;//crc
	AllBuf.Tx2_1[m++]=CRCLow;		
	 
	U2DS.TLong=m;    
	U2DS.SedOrd = 'E';  //;串口1发送命令允许
	U2DS.BufAdr = AllBuf.Tx2_1;//将发送缓冲区的数据发送到变频器      
}
}
/*----------------------------------------------------------------------------*
 |  串口函数
 *----------------------------------------------------------------------------*/
void USART_Com(void)
{  
    USART2_Rece();
    USART2_Send();   
}
/*----------------------------------------------------------------------------*
**串口的通讯
 *----------------------------------------------------------------------------*/
void ZD_Port(void)  
{
		u8 i=0;
		u8 len=0;			
		//***********************串口2的通讯**********************//
    if(U2DS.RecOrd == true) //;串口2接收到数据
    {
        U2DS.RecOrd = false;
				for(i=0;i<U2DS.RecLen;i++)			
					AllBuf.Rx2_2[i]=*(U2DS.BufAdr+i);
				switch(AllBuf.Rx2_2[0])//判断地址
				{
					case 0x01://上夹具的计数器							
					case 0x03://下夹具的计数器
						if(U2DS.RecLen==29)
						{
							len=27;
 							Cal_CRC(AllBuf.Rx2_2,len);	// 进行 CRC16 校验
 							if((CRCHigh == AllBuf.Rx2_2[len])&&(CRCLow == AllBuf.Rx2_2[len+1]))	// CRC校验正确,接收数据有效							
								U2DS.RecStatus='R';//接收到数据	
						}	
					else if(U2DS.RecLen==8)
					{
						len=6;
						Cal_CRC(AllBuf.Rx2_2,len);	// 进行 CRC16 校验
						if((CRCHigh == AllBuf.Rx2_2[len])&&(CRCLow == AllBuf.Rx2_2[len+1])&&AllBuf.Rx2_2[0]==0x01&&AllBuf.Rx2_2[4]==0xff)//上游start
							CLJ_START=1;
						else if((CRCHigh == AllBuf.Rx2_2[len])&&(CRCLow == AllBuf.Rx2_2[len+1])&&AllBuf.Rx2_2[0]==0x01&&AllBuf.Rx2_2[4]==0x00)//上游off
							CLJ_close=1;
						else if((CRCHigh == AllBuf.Rx2_2[len])&&(CRCLow == AllBuf.Rx2_2[len+1])&&AllBuf.Rx2_2[0]==0x03&&AllBuf.Rx2_2[4]==0xff)//下游start
							CLJ_START=2;
						else if((CRCHigh == AllBuf.Rx2_2[len])&&(CRCLow == AllBuf.Rx2_2[len+1])&&AllBuf.Rx2_2[0]==0x03&&AllBuf.Rx2_2[4]==0x00)//下游off
							CLJ_close=2;	
					}	
							break;
					case 0x05:	//流量计
						len=7;
						Cal_CRC(AllBuf.Rx2_2,len);	// 进行 CRC16 校验	
						if((CRCHigh == AllBuf.Rx2_2[len])&&(CRCLow == AllBuf.Rx2_2[len+1]))	// CRC校验正确,接收数据有效
							U2DS.RecStatus='R';//接收到数据
							break;				
					default:break;
				}
    }
}

/*----------------------------------------------------------------------------*
**对串口2收到的数据进行处理
 *----------------------------------------------------------------------------*/
void DataProcess(void)
{	
	u16 flow1=0,flow2=0;
	
	//***********************对串口2收到的数据进行处理**********************//
		if(U2DS.RecStatus=='R')
		{	
			U2DS.RecStatus=0;
			switch(AllBuf.Rx2_2[0])	// 判断地址码
			{	
				case 0x01://上夹具尘埃粒子计数器
						Dust_03_Up=(((u32)AllBuf.Rx2_2[3]<<24)+((u32)AllBuf.Rx2_2[4]<<16)+((u16)AllBuf.Rx2_2[5]<<8)+(AllBuf.Rx2_2[6]));
						Dust_05_Up=(((u32)AllBuf.Rx2_2[7]<<24)+((u32)AllBuf.Rx2_2[8]<<16)+((u16)AllBuf.Rx2_2[9]<<8)+(AllBuf.Rx2_2[10]));
						Dust_1_Up= (((u32)AllBuf.Rx2_2[11]<<24)+((u32)AllBuf.Rx2_2[12]<<16)+((u16)AllBuf.Rx2_2[13]<<8)+(AllBuf.Rx2_2[14]));
						Dust_3_Up= (((u32)AllBuf.Rx2_2[15]<<24)+((u32)AllBuf.Rx2_2[16]<<16)+((u16)AllBuf.Rx2_2[17]<<8)+(AllBuf.Rx2_2[18]));
						Dust_5_Up= (((u32)AllBuf.Rx2_2[19]<<24)+((u32)AllBuf.Rx2_2[20]<<16)+((u16)AllBuf.Rx2_2[21]<<8)+(AllBuf.Rx2_2[22]));
						Dust_10_Up=(((u32)AllBuf.Rx2_2[23]<<24)+((u32)AllBuf.Rx2_2[24]<<16)+((u16)AllBuf.Rx2_2[25]<<8)+(AllBuf.Rx2_2[26]));
						if(start_testflag==1)//按下开始测试按钮
						{
							RTC_12000MS=0;						
						}
						break;
				case 0x03://下夹具尘埃粒子计数器				
						Dust_03_Down=(((u32)AllBuf.Rx2_2[3]<<24)+((u32)AllBuf.Rx2_2[4]<<16)+((u16)AllBuf.Rx2_2[5]<<8)+(AllBuf.Rx2_2[6]));
						Dust_05_Down=(((u32)AllBuf.Rx2_2[7]<<24)+((u32)AllBuf.Rx2_2[8]<<16)+((u16)AllBuf.Rx2_2[9]<<8)+(AllBuf.Rx2_2[10]));
						Dust_1_Down= (((u32)AllBuf.Rx2_2[11]<<24)+((u32)AllBuf.Rx2_2[12]<<16)+((u16)AllBuf.Rx2_2[13]<<8)+(AllBuf.Rx2_2[14]));
						Dust_3_Down= (((u32)AllBuf.Rx2_2[15]<<24)+((u32)AllBuf.Rx2_2[16]<<16)+((u16)AllBuf.Rx2_2[17]<<8)+(AllBuf.Rx2_2[18]));
// 						Dust_5_Down= (((u32)AllBuf.Rx2_2[19]<<24)+((u32)AllBuf.Rx2_2[20]<<16)+((u16)AllBuf.Rx2_2[21]<<8)+(AllBuf.Rx2_2[22]));
// 						Dust_10_Down=(((u32)AllBuf.Rx2_2[23]<<24)+((u32)AllBuf.Rx2_2[24]<<16)+((u16)AllBuf.Rx2_2[25]<<8)+(AllBuf.Rx2_2[26]));
						Dust_5_Down=0;
						Dust_10_Down=0;
						Data_Time_down++;//下夹具读数次数
						if(start_testflag==1)//按下开始测试按钮
						{
							RTC_13000MS=0;						
						}
						if(Data_Time_down==6)//取样完成，计算出过滤效率
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
				case 0x05://流量计
						flow1=AllBuf.Rx2_2[3]*256+AllBuf.Rx2_2[4];
						flow2=AllBuf.Rx2_2[5]*256+AllBuf.Rx2_2[6];
						Flow=(float)(flow1*65535+flow2)/1000;
						
					break;				
			}
				
		}
}

