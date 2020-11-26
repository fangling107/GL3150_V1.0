#include "ft5206.h"
#include "ctiic.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//7����ݴ�����-FT5206 ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/11/26
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2015-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
ft_ctp_dev ctp_dev=
{
    FT5206_Init,
    FT5206_Scan,
    0,
    0,
    0,
    0,
};
 /*********************************/
//�жϰ����Ƿ��ɿ�
//��δ�ɿ�����ȴ��ɿ�
//���ɿ��������
/*********************************/
void Loosen_Lcd(void)
{
	u8 i = 1;
	while(i)
	{
		ctp_dev.scan();
		if (ctp_dev.tpsta&TP_PRES_DOWN)
			i = 1;
		else	
			i = 0;
	}	
}   
//��FT5206д��һ������
//reg:��ʼ�Ĵ�����ַ
//buf:���ݻ�������
//len:д���ݳ���
//����ֵ:0,�ɹ�;1,ʧ��.
u8 FT5206_WR_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i;
	u8 ret=0;
	CT_IIC_Start();	 
	CT_IIC_Send_Byte(FT_CMD_WR);	//����д���� 	 
	CT_IIC_Wait_Ack(); 	 										  		   
	CT_IIC_Send_Byte(reg&0XFF);   	//���͵�8λ��ַ
	CT_IIC_Wait_Ack();  
	for(i=0;i<len;i++)
	{	   
    	CT_IIC_Send_Byte(buf[i]);  	//������
		ret=CT_IIC_Wait_Ack();
		if(ret)break;  
	}
    CT_IIC_Stop();					//����һ��ֹͣ����	    
	return ret; 
}
//��FT5206����һ������
//reg:��ʼ�Ĵ�����ַ
//buf:���ݻ�������
//len:�����ݳ���			  
void FT5206_RD_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i; 
 	CT_IIC_Start();	
 	CT_IIC_Send_Byte(FT_CMD_WR);   	//����д���� 	 
	CT_IIC_Wait_Ack(); 	 										  		   
 	CT_IIC_Send_Byte(reg&0XFF);   	//���͵�8λ��ַ
	CT_IIC_Wait_Ack();  
 	CT_IIC_Start();  	 	   
	CT_IIC_Send_Byte(FT_CMD_RD);   	//���Ͷ�����		   
	CT_IIC_Wait_Ack();	   
	for(i=0;i<len;i++)
	{	   
    	buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //������	  
	} 
    CT_IIC_Stop();//����һ��ֹͣ����     
} 
//��ʼ��FT5206������
//����ֵ:0,��ʼ���ɹ�;1,��ʼ��ʧ�� 
u8 FT5206_Init(void)
{
	u8 temp[2]; 
 	RCC->APB2ENR|=1<<7;		//��ʹ������IO PORTFʱ��  
	RCC->APB2ENR|=1<<3;		//��ʹ������IO PORTBʱ��  wo jia
	
//	GPIOF->CRH&=0XFFFF00FF;	
//	GPIOF->CRH|=0X00003800;	//PF10 ���� PF11�������   
//	GPIOF->ODR|=3<<10;	   	//PF10/11 ����/���1 
	GPIOB->CRL&=0XFFFFF0FF;	//PB2 �������
	GPIOB->CRL|=0X00000300;	   
	GPIOB->ODR|=1<<2;	    //PB2 �����
	
	GPIOF->CRH&=0XFFFFF0FF;	
	GPIOF->CRH|=0X00000800;	//PF10 ����
	GPIOF->ODR|=1<<10;	   	//PF10����
	
/*	GPIOF->CRH&=0XFFFF00FF;	
	GPIOF->CRH|=0X00003800;	//PF10 ���� PF11�������   
	GPIOF->ODR|=3<<10;	   	//PF10/11 ����/���1 */
	
	CT_IIC_Init();      	//��ʼ����������I2C����  
	FT_RST=0;				//��λ
	delay_ms(20);
 	FT_RST=1;				//�ͷŸ�λ		    
	delay_ms(50);  	
	temp[0]=0;
	FT5206_WR_Reg(FT_DEVIDE_MODE,temp,1);	//������������ģʽ 
	FT5206_WR_Reg(FT_ID_G_MODE,temp,1);		//��ѯģʽ 
	temp[0]=22;								//������Чֵ��22��ԽСԽ����	
	FT5206_WR_Reg(FT_ID_G_THGROUP,temp,1);	//���ô�����Чֵ
	temp[0]=12;								//�������ڣ�����С��12�����14
	FT5206_WR_Reg(FT_ID_G_PERIODACTIVE,temp,1); 
	//��ȡ�汾�ţ��ο�ֵ��0x3003
	FT5206_RD_Reg(FT_ID_G_LIB_VERSION,&temp[0],2);  
	if((temp[0]==0X30||temp[0]==0X00)&&(temp[1]==0X03||temp[1]==0X01))//�汾:0X3003
	{
		//printf("CTP ID:%x\r\n",((u16)temp[0]<<8)+temp[1]);
		return 0;
	} 
//	return 1;//return 1���ܳ�ʼ��
	return 1;
}
const u16 FT5206_TPX_TBL[5]={FT_TP1_REG,FT_TP2_REG,FT_TP3_REG,FT_TP4_REG,FT_TP5_REG};
//ɨ�败����(���ò�ѯ��ʽ)
//mode:0,����ɨ��.
//����ֵ:��ǰ����״̬.
//0,�����޴���;1,�����д���
//u8 FT5206_Scan(u8 mode)
u8 FT5206_Scan(void)
{
	u8 buf[4];
	u8 i=0;
	u8 res=0;
	u8 temp;
	u8 mode = 0;
	static u8 t=0;//���Ʋ�ѯ���,�Ӷ�����CPUռ����   
	t++;
	if((t%10)==0||t<10)//����ʱ,ÿ����10��CTP_Scan�����ż��1��,�Ӷ���ʡCPUʹ����
	{
		FT5206_RD_Reg(FT_REG_NUM_FINGER,&mode,1);//��ȡ�������״̬  
		if((mode&0XF)&&((mode&0XF)<6))
		{
			temp=0XFF<<(mode&0XF);//����ĸ���ת��Ϊ1��λ��,ƥ��tp_dev.sta���� 
			ctp_dev.tpsta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES; 
			for(i=0;i<5;i++)
			{
				if(ctp_dev.tpsta&(1<<i))	//������Ч?
				{
					FT5206_RD_Reg(FT5206_TPX_TBL[i],buf,4);	//��ȡXY����ֵ 
					ctp_dev.y[i]=((u16)(buf[0]&0X0F)<<8)+buf[1];
					ctp_dev.x[i]=((u16)(buf[2]&0X0F)<<8)+buf[3];  
					if((buf[0]&0XF0)!=0X80)ctp_dev.x[i]=ctp_dev.y[i]=0;//������contact�¼�������Ϊ��Ч					
				}			
			} 
			res=1;
			if(ctp_dev.x[0]==0 && ctp_dev.y[0]==0)mode=0;	//���������ݶ���0,����Դ˴�����
			t=0;		//����һ��,��������������10��,�Ӷ����������
		}
	}
	if((mode&0X1F)==0)//�޴����㰴��
	{ 
		if(ctp_dev.tpsta&TP_PRES_DOWN)	//֮ǰ�Ǳ����µ�
		{
			ctp_dev.tpsta&=~(1<<7);	//��ǰ����ɿ�
		}else						//֮ǰ��û�б�����
		{ 
			ctp_dev.x[0]=0xffff;
			ctp_dev.y[0]=0xffff;
			ctp_dev.tpsta&=0XE0;	//�������Ч���	
		}	 
	} 	
	if(t>240)t=10;//���´�10��ʼ����
	return res;
}


//ɨ��GT811
//�õ��Ľ��������ctp_dev�ṹ����								   									 
void FT5206_Scan1(void)
{			   
	u8 buf[34];//һ�ζ�ȡ34�ֽ�
	u8 i=0;
	if((ctp_dev.tpsta&0X80)==0)return;			//����Ч����,���ȡ����,����ֱ���˳�
  	FT5206_RD_Reg(FT5206_TPX_TBL[i],buf,34);		//һ�ζ�ȡ34���ֽ�
	ctp_dev.tpsta=buf[0]&0X1F;					//��������λ,ͬʱ�����Ч�������
#if CT_EXCHG_XY==1								//��תXY
	ctp_dev.y[0]=480-(((u16)buf[2]<<8)+buf[3]);	//������0����
	ctp_dev.x[0]=((u16)buf[4]<<8)+buf[5];	  
	ctp_dev.ppr[0]=buf[6];	 
	ctp_dev.y[1]=480-(((u16)buf[7]<<8)+buf[8]);	//������1����
	ctp_dev.x[1]=((u16)buf[9]<<8)+buf[10];	  
	ctp_dev.ppr[1]=buf[11];
	ctp_dev.y[2]=480-(((u16)buf[12]<<8)+buf[13]);//������2����
	ctp_dev.x[2]=((u16)buf[14]<<8)+buf[15];	  
	ctp_dev.ppr[2]=buf[16];					   
	ctp_dev.y[3]=480-(((u16)buf[17]<<8)+buf[24]);//������3����
	ctp_dev.x[3]=((u16)buf[25]<<8)+buf[26];	  
	ctp_dev.ppr[3]=buf[27];
	ctp_dev.y[4]=480-(((u16)buf[28]<<8)+buf[29]);//������4����
	ctp_dev.x[4]=((u16)buf[30]<<8)+buf[31];	  
	ctp_dev.ppr[4]=buf[32];
#else 
	ctp_dev.y[0]=((u16)buf[2]<<8)+buf[3];		//������0����
	ctp_dev.x[0]=800-(((u16)buf[4]<<8)+buf[5]);	  
//	ctp_dev.ppr[0]=buf[6];	 
	ctp_dev.y[1]=((u16)buf[7]<<8)+buf[8];		//������1����
	ctp_dev.x[1]=800-(((u16)buf[9]<<8)+buf[10]);	  
//	ctp_dev.ppr[1]=buf[11];
	ctp_dev.y[2]=((u16)buf[12]<<8)+buf[13];		//������2����
	ctp_dev.x[2]=800-(((u16)buf[14]<<8)+buf[15]);	  
//	ctp_dev.ppr[2]=buf[16];					   
	ctp_dev.y[3]=((u16)buf[17]<<8)+buf[24];		//������3����
	ctp_dev.x[3]=800-(((u16)buf[25]<<8)+buf[26]);	  
//	ctp_dev.ppr[3]=buf[27];
	ctp_dev.y[4]=((u16)buf[28]<<8)+buf[29];		//������4����
	ctp_dev.x[4]=800-(((u16)buf[30]<<8)+buf[31]);	  
//	ctp_dev.ppr[4]=buf[32];
#endif	 		    
}

























