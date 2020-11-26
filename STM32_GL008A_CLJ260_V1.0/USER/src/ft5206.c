#include "ft5206.h"
#include "ctiic.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//7寸电容触摸屏-FT5206 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/11/26
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2015-2024
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
//判断按键是否松开
//若未松开，则等待松开
//若松开，则继续
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
//向FT5206写入一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:写数据长度
//返回值:0,成功;1,失败.
u8 FT5206_WR_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i;
	u8 ret=0;
	CT_IIC_Start();	 
	CT_IIC_Send_Byte(FT_CMD_WR);	//发送写命令 	 
	CT_IIC_Wait_Ack(); 	 										  		   
	CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
	CT_IIC_Wait_Ack();  
	for(i=0;i<len;i++)
	{	   
    	CT_IIC_Send_Byte(buf[i]);  	//发数据
		ret=CT_IIC_Wait_Ack();
		if(ret)break;  
	}
    CT_IIC_Stop();					//产生一个停止条件	    
	return ret; 
}
//从FT5206读出一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:读数据长度			  
void FT5206_RD_Reg(u16 reg,u8 *buf,u8 len)
{
	u8 i; 
 	CT_IIC_Start();	
 	CT_IIC_Send_Byte(FT_CMD_WR);   	//发送写命令 	 
	CT_IIC_Wait_Ack(); 	 										  		   
 	CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
	CT_IIC_Wait_Ack();  
 	CT_IIC_Start();  	 	   
	CT_IIC_Send_Byte(FT_CMD_RD);   	//发送读命令		   
	CT_IIC_Wait_Ack();	   
	for(i=0;i<len;i++)
	{	   
    	buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //发数据	  
	} 
    CT_IIC_Stop();//产生一个停止条件     
} 
//初始化FT5206触摸屏
//返回值:0,初始化成功;1,初始化失败 
u8 FT5206_Init(void)
{
	u8 temp[2]; 
 	RCC->APB2ENR|=1<<7;		//先使能外设IO PORTF时钟  
	RCC->APB2ENR|=1<<3;		//先使能外设IO PORTB时钟  wo jia
	
//	GPIOF->CRH&=0XFFFF00FF;	
//	GPIOF->CRH|=0X00003800;	//PF10 输入 PF11推挽输出   
//	GPIOF->ODR|=3<<10;	   	//PF10/11 上拉/输出1 
	GPIOB->CRL&=0XFFFFF0FF;	//PB2 推挽输出
	GPIOB->CRL|=0X00000300;	   
	GPIOB->ODR|=1<<2;	    //PB2 输出高
	
	GPIOF->CRH&=0XFFFFF0FF;	
	GPIOF->CRH|=0X00000800;	//PF10 输入
	GPIOF->ODR|=1<<10;	   	//PF10上拉
	
/*	GPIOF->CRH&=0XFFFF00FF;	
	GPIOF->CRH|=0X00003800;	//PF10 输入 PF11推挽输出   
	GPIOF->ODR|=3<<10;	   	//PF10/11 上拉/输出1 */
	
	CT_IIC_Init();      	//初始化电容屏的I2C总线  
	FT_RST=0;				//复位
	delay_ms(20);
 	FT_RST=1;				//释放复位		    
	delay_ms(50);  	
	temp[0]=0;
	FT5206_WR_Reg(FT_DEVIDE_MODE,temp,1);	//进入正常操作模式 
	FT5206_WR_Reg(FT_ID_G_MODE,temp,1);		//查询模式 
	temp[0]=22;								//触摸有效值，22，越小越灵敏	
	FT5206_WR_Reg(FT_ID_G_THGROUP,temp,1);	//设置触摸有效值
	temp[0]=12;								//激活周期，不能小于12，最大14
	FT5206_WR_Reg(FT_ID_G_PERIODACTIVE,temp,1); 
	//读取版本号，参考值：0x3003
	FT5206_RD_Reg(FT_ID_G_LIB_VERSION,&temp[0],2);  
	if((temp[0]==0X30||temp[0]==0X00)&&(temp[1]==0X03||temp[1]==0X01))//版本:0X3003
	{
		//printf("CTP ID:%x\r\n",((u16)temp[0]<<8)+temp[1]);
		return 0;
	} 
//	return 1;//return 1不能初始化
	return 1;
}
const u16 FT5206_TPX_TBL[5]={FT_TP1_REG,FT_TP2_REG,FT_TP3_REG,FT_TP4_REG,FT_TP5_REG};
//扫描触摸屏(采用查询方式)
//mode:0,正常扫描.
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
//u8 FT5206_Scan(u8 mode)
u8 FT5206_Scan(void)
{
	u8 buf[4];
	u8 i=0;
	u8 res=0;
	u8 temp;
	u8 mode = 0;
	static u8 t=0;//控制查询间隔,从而降低CPU占用率   
	t++;
	if((t%10)==0||t<10)//空闲时,每进入10次CTP_Scan函数才检测1次,从而节省CPU使用率
	{
		FT5206_RD_Reg(FT_REG_NUM_FINGER,&mode,1);//读取触摸点的状态  
		if((mode&0XF)&&((mode&0XF)<6))
		{
			temp=0XFF<<(mode&0XF);//将点的个数转换为1的位数,匹配tp_dev.sta定义 
			ctp_dev.tpsta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES; 
			for(i=0;i<5;i++)
			{
				if(ctp_dev.tpsta&(1<<i))	//触摸有效?
				{
					FT5206_RD_Reg(FT5206_TPX_TBL[i],buf,4);	//读取XY坐标值 
					ctp_dev.y[i]=((u16)(buf[0]&0X0F)<<8)+buf[1];
					ctp_dev.x[i]=((u16)(buf[2]&0X0F)<<8)+buf[3];  
					if((buf[0]&0XF0)!=0X80)ctp_dev.x[i]=ctp_dev.y[i]=0;//必须是contact事件，才认为有效					
				}			
			} 
			res=1;
			if(ctp_dev.x[0]==0 && ctp_dev.y[0]==0)mode=0;	//读到的数据都是0,则忽略此次数据
			t=0;		//触发一次,则会最少连续监测10次,从而提高命中率
		}
	}
	if((mode&0X1F)==0)//无触摸点按下
	{ 
		if(ctp_dev.tpsta&TP_PRES_DOWN)	//之前是被按下的
		{
			ctp_dev.tpsta&=~(1<<7);	//标记按键松开
		}else						//之前就没有被按下
		{ 
			ctp_dev.x[0]=0xffff;
			ctp_dev.y[0]=0xffff;
			ctp_dev.tpsta&=0XE0;	//清除点有效标记	
		}	 
	} 	
	if(t>240)t=10;//重新从10开始计数
	return res;
}


//扫描GT811
//得到的结果保存在ctp_dev结构体内								   									 
void FT5206_Scan1(void)
{			   
	u8 buf[34];//一次读取34字节
	u8 i=0;
	if((ctp_dev.tpsta&0X80)==0)return;			//有有效触摸,则读取数据,否则直接退出
  	FT5206_RD_Reg(FT5206_TPX_TBL[i],buf,34);		//一次读取34个字节
	ctp_dev.tpsta=buf[0]&0X1F;					//触摸点标记位,同时清除有效触摸标记
#if CT_EXCHG_XY==1								//调转XY
	ctp_dev.y[0]=480-(((u16)buf[2]<<8)+buf[3]);	//触摸点0坐标
	ctp_dev.x[0]=((u16)buf[4]<<8)+buf[5];	  
	ctp_dev.ppr[0]=buf[6];	 
	ctp_dev.y[1]=480-(((u16)buf[7]<<8)+buf[8]);	//触摸点1坐标
	ctp_dev.x[1]=((u16)buf[9]<<8)+buf[10];	  
	ctp_dev.ppr[1]=buf[11];
	ctp_dev.y[2]=480-(((u16)buf[12]<<8)+buf[13]);//触摸点2坐标
	ctp_dev.x[2]=((u16)buf[14]<<8)+buf[15];	  
	ctp_dev.ppr[2]=buf[16];					   
	ctp_dev.y[3]=480-(((u16)buf[17]<<8)+buf[24]);//触摸点3坐标
	ctp_dev.x[3]=((u16)buf[25]<<8)+buf[26];	  
	ctp_dev.ppr[3]=buf[27];
	ctp_dev.y[4]=480-(((u16)buf[28]<<8)+buf[29]);//触摸点4坐标
	ctp_dev.x[4]=((u16)buf[30]<<8)+buf[31];	  
	ctp_dev.ppr[4]=buf[32];
#else 
	ctp_dev.y[0]=((u16)buf[2]<<8)+buf[3];		//触摸点0坐标
	ctp_dev.x[0]=800-(((u16)buf[4]<<8)+buf[5]);	  
//	ctp_dev.ppr[0]=buf[6];	 
	ctp_dev.y[1]=((u16)buf[7]<<8)+buf[8];		//触摸点1坐标
	ctp_dev.x[1]=800-(((u16)buf[9]<<8)+buf[10]);	  
//	ctp_dev.ppr[1]=buf[11];
	ctp_dev.y[2]=((u16)buf[12]<<8)+buf[13];		//触摸点2坐标
	ctp_dev.x[2]=800-(((u16)buf[14]<<8)+buf[15]);	  
//	ctp_dev.ppr[2]=buf[16];					   
	ctp_dev.y[3]=((u16)buf[17]<<8)+buf[24];		//触摸点3坐标
	ctp_dev.x[3]=800-(((u16)buf[25]<<8)+buf[26]);	  
//	ctp_dev.ppr[3]=buf[27];
	ctp_dev.y[4]=((u16)buf[28]<<8)+buf[29];		//触摸点4坐标
	ctp_dev.x[4]=800-(((u16)buf[30]<<8)+buf[31]);	  
//	ctp_dev.ppr[4]=buf[32];
#endif	 		    
}

























