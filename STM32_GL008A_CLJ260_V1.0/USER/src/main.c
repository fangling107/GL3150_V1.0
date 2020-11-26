#include "stm32f10x.h"
#include "setup.h"
#include "24cxx.h" 
#include "flash.h" 
#include "picture.h"
#include "rtc.h" 
#include "ads8341.h"
#include "printer.h"
#include "string.h"
#include "stdio.h"
#include "lcd.h"
#include "ft5206.h"
#include "Serial.h"
//定义EEPROM的保存地址 0-13 屏幕校准系数，电阻屏需要使用
//14：试剂系数 孔径测试
//15：测试次数 孔径测试
//16：测试组数 孔径测试
//17-18：AD K整数部分 19-20：AD K小数部分
//21-22: AD C整数部分 23-24：AD C小数部分
//25-26:	标定点1			27-28：AD1
//29-30:	标定点2			31-32：AD
// #define SAVE_ADDR_XISHU 14 //EEPROM保存系数的地址 14开始 （0-13保存的是屏幕校准系数）
// #define SAVE_ADDR_CISHU 15	//EEPROM保存测试次数的地址 15开始 
// #define SAVE_ADDR_ZUSHU 16	//EEPROM保存测试组数的地址 16开始
#define SAVE_ADDR_Num 33
//AD校准密码
#define password 70605

//计数器地址
 u8 CLJ_R260_UpAddr=0x01;		//上夹具尘埃粒子计数器的地址(宏瑞) 485接口
 u8 CLJ_R260_DownAddr=0x03;	//下夹具尘埃粒子计数器的地址(宏瑞) 485接口
 u8 Flow_meter=0x05;					//流量计的地址									 485接口

//参数声明
static u8 flow_flag;//流量选择标志
u8	Flow_set=32;//设置流量
static u8 model_flag;//模式选择标志
static u8 key_value;//按键键值
static u8 set_numkey;//设置时间日期编号的键值
static u16 number;//操作员编号
static u8	Test_num;//试样编号
static u8 passwordcorrect;//密码正确标志
u8 start_testflag=0;//启动测试标志
u8 Test_over;//测试完成标志
u8 No_Load;//空载标志
static u8 Stop_Flag;//测试停止标志
static u8 Error_Flag;//出现异常标志
static u8 Error_Num_Flow;//流量异常次数
u8 No_Loadorclear_Flag=0;
extern u16 OS_TimeMS;
extern float press_k;//曲线斜率
extern float press_c;//曲线偏移
//250ms標誌
u8 RTC_250MS=0;
u8 RTC_500MS=0;
u8 RTC_12000MS=0;
u8 RTC_13000MS=0;
u8 RTC_2500MS=0;

u8 CLJ_START=0;//CLJ启动
u8 CLJ_close=0;//CLJ关闭
u8 Data_Back=0;//数据返回成功
u16 pressAD_kint,pressAD_kfloat;//曲线系数的整数和小数部分
u16 pressAD_cint,pressAD_cfloat;//曲线系数的整数和小数部分
u16 AD1value=0,AD2value=0,biaoding1=0,biaoding2=0;//AD采样值
u16 press_zeroAD=0;//零点AD
u8 Test_succesflag=0;//测试成功标志
u16 pressADvalue;//AD值
u16 press=0;//测得的压力值
float Flow=0;//测得的流量值
u8 Data_checkNum=1;//数据查询
u8 Press_Flag=0;//压力标志
u8 Press_DataSave[10]={0};
u8 Data_Time_down=0;
//上游颗粒数
u32	Dust_03_Up=0;
u32	Dust_05_Up=0;
u32	Dust_1_Up=0;
u32	Dust_3_Up=0;
u32	Dust_5_Up=0;
u32	Dust_10_Up=0;
//下游颗粒数
u32	Dust_03_Down=0;
u32	Dust_05_Down=0;
u32	Dust_1_Down=0;
u32	Dust_3_Down=0;
u32	Dust_5_Down=0;
u32	Dust_10_Down=0;

//过滤效率
float Efficiency_03=0;
float Efficiency_05=0;
float Efficiency_1=0;
float Efficiency_3=0;
float Efficiency_5=0;
float Efficiency_10=0;

struct Data_Dust	Dust;//一组数据的结构体

//函数声明
void menu_start(void);
void menu1(void);//主界面
void menu_setting(void);//设置界面
void menu_setkeyboard(void);//键盘设置界面
void menu_help(void);//帮助界面
void menu_Calibration(void);//校准界面
void key_function(void);
void display_number(void);
void JianPan_Displayfu(void);//显示虚拟键盘
void PrintReport(void);
void speekers(u16 time);//蜂鸣器 time/1ms
void AD_Calibration_menu(void);
void menu_setting_key(void);
void menu_flashmax(void);//数据区满提示
void display_pressspeed(void);
u16 paixu(u8 times,u16 rcv_data[]);
void menu_password(void);
void display_time(void);
void menu_start_keytest(void);
void menu_datacheck(void);
void menu_datacheck_key(void);
void menu_pressError(void);//测试阻力超限提示界面
void menu_FlowError(void);//测试流量超限提示界面
void Test_Function(void);
void Test_DataSave(void);
void Data_Fullmenu(void);//提示数据满
void Zero_Fail(void);
//主函数
int main(void)
 {
	u8 b[8]={0};	
  InitHardware();										//硬件初始化
	while(RTC_Init())	;								//RTC初始化
	AT24CXX_Init();										//AT24C02初始化
	while(AT24CXX_Check());						//检测24c02	
	Printer_Init();										//打印端口初始化	
	SPI_Flash_Init();									//W25Q16初始化				
	SPI_AD8341_Init();								//AD8341初始化
	LCD_Init();
	while(ctp_dev.init())		   	//初始化电容触摸屏
	{
		Show_Str(60,110,lcddev.width,32,"ctouch Error",32,0); 
		delay_ms(20);
		Show_Str(60,110,lcddev.width,32,"please check",32,0); 
 		delay_ms(20);		
	}
	Adc_Init();
	//读取保存的压力传感器AD系数
	AT24CXX_Read(b,17,8);
	pressAD_kint=(b[0]<<8)+b[1];
	pressAD_kfloat=(b[2]<<8)+b[3];//k
	pressAD_cint=(b[4]<<8)+b[5];
	pressAD_cfloat=(b[6]<<8)+b[7];//c
	press_k=pressAD_kint+((float)pressAD_kfloat/10000);
	press_c=pressAD_cint+((float)pressAD_cfloat/10000);
	//读取系数和试样编号
	Test_num=AT24CXX_ReadOneByte(SAVE_ADDR_Num);
	if(Test_num==0xff) Test_num=1;
	//零点
	press_zeroAD=Get_Adc_Average(10, 50);
	speekers(500);//蜂鸣器响500mS	
	menu_start_keytest();//加载初始界面和按键检测	
	while(1)
	{			
		USART_Com();        //;串口通讯		
		ZD_Port();					//;终端端口通讯 
		DataProcess();			//;终端数据处理及数据发送	
		key_function();//主界面的按键功能函数
		Test_Function();
		Test_DataSave();//数据保存
		
	}
}
//开始测试后进行数据发送
void Test_Function(void)
{
	if(start_testflag==1)//按下开始测试按钮
		{	
			if(RTC_250MS&&CLJ_START==0)//启动上游计数器
			{
				RTC_250MS=0;					
				U2DS.SedOrd = 0;	
				Sendout_uart1_CLJ260(CLJ_R260_UpAddr,0x02);		//启动上游计数器开始测量
			}
			else if(RTC_250MS&&CLJ_START==1)//启动下游计数器
			{
				RTC_250MS=0;			
				U2DS.SedOrd = 0	;
				Sendout_uart1_CLJ260(CLJ_R260_DownAddr,0x02);	//启动下游计数器开始测量
			}	
			if(RTC_250MS&&RTC_12000MS&&CLJ_START==2&&CLJ_close==0)//每10S读取1次上下游计数器的值和流量的值
			{		
				RTC_250MS=0;	
				U2DS.SedOrd = 0	;	
				Sendout_uart1_CLJ260(CLJ_R260_UpAddr,0x01);			
			}
			else if(RTC_250MS&&RTC_13000MS&&CLJ_START==2&&CLJ_close==0)
			{	
				RTC_250MS=0;	
				U2DS.SedOrd = 0	;	
				Sendout_uart1_CLJ260(CLJ_R260_DownAddr,0x01);//读取上下游的离子浓度			
			}		
			if(Test_over==1||Stop_Flag==1)//测试完成标志
			{
				if(CLJ_close<2)
				{
					if(RTC_250MS&&CLJ_close==0)
					{
						RTC_250MS=0;								
						U2DS.SedOrd = 0	;	
						Sendout_uart1_CLJ260(CLJ_R260_UpAddr,0x03);		//关闭上游计数器开始测量
					}
					else if(RTC_250MS&&CLJ_close==1)
					{
						RTC_250MS=0;
						U2DS.SedOrd = 0	;
						Sendout_uart1_CLJ260(CLJ_R260_DownAddr,0x03);	//关闭下游计数器开始测量
					}
				}
				else
				{				
					Stop_Flag=0;					
					start_testflag=0;
					Data_Time_down=0;
					if(Test_over)
					{
						Test_over=0;
						Test_succesflag=1;//测试成功标志						
					}
					if(model_flag==0)//NACL
					{Nacl_OFF;}
					else//DEHS
					{DEHS_OFF;}	
					CLJ_START=0;
					CLJ_close=0;					
					MOTOR_OFF;//关闭气泵电源
					Air_cylinder_OFF;//气缸断电，夹具升起来					
					menu1();//加载初始菜单界面						
				}
			}
		}
}
//测试数据保存
void Test_DataSave(void)
{
	u16 flow_data=0;
	if(Test_succesflag)
	{
		Test_succesflag=0;
		Dust.Data_years[0]=calendar.w_year>>8;
		Dust.Data_years[1]=calendar.w_year;		
		Dust.Data_month=calendar.w_month;		
		Dust.Data_day=calendar.w_date;		
		Dust.Hours=calendar.hour;
		Dust.Minutes=calendar.min;
		Dust.Seconds=calendar.sec;
		
		Dust.People[0]=number>>8;
		Dust.People[1]=number;
		
		Dust.num=Test_num;		
		
		Dust.Press[0]=press>>8;
		Dust.Press[1]=press;		
	
		Dust.Flow=flow_flag;		
		Dust.Aerosol=model_flag;//气溶胶选择
		
		flow_data=Efficiency_03*100;
		Dust.Efficiency_03[0]=flow_data>>8;
		Dust.Efficiency_03[1]=flow_data;
		
		flow_data=Efficiency_05*100;
		Dust.Efficiency_05[0]=flow_data>>8;
		Dust.Efficiency_05[1]=flow_data;
		
		flow_data=Efficiency_1*100;
		Dust.Efficiency_1[0]=flow_data>>8;
		Dust.Efficiency_1[1]=flow_data;
		
		flow_data=Efficiency_3*100;
		Dust.Efficiency_3[0]=flow_data>>8;
		Dust.Efficiency_3[1]=flow_data;
		
		flow_data=Efficiency_5*100;
		Dust.Efficiency_5[0]=flow_data>>8;
		Dust.Efficiency_5[1]=flow_data;
		
		flow_data=Efficiency_10*100;
		Dust.Efficiency_10[0]=flow_data>>8;
		Dust.Efficiency_10[1]=flow_data;
		
		//保存到flash		
		SPI_Flash_Write(&Dust.Data_years[0],(Test_num-1)*256,26);
		Test_num++;
		AT24CXX_WriteOneByte(SAVE_ADDR_Num,Test_num);	
	}
}
//指定位置显示上下游颗粒数和计算出过滤效率
void display_Dust(void)
{	
	u16 TU16_BackColor;

	char c[5]={0};
	//颗粒数
	TU16_BackColor = BACK_COLOR;	
	BACK_COLOR=LIGHTGRAY;
	//上游
																	
	LCD_ShowxNum(496,258,Dust_03_Up,8,16,0x80);//0.3													
	LCD_ShowxNum(496,276,Dust_05_Up,8,16,0x80);//0.5															
	LCD_ShowxNum(496,294,Dust_1_Up,8,16,0x80);//1.0
	LCD_ShowxNum(496,312,Dust_3_Up,8,16,0x80);//3.0	
	LCD_ShowxNum(496,330,Dust_5_Up,8,16,0x80);//5.0															
	LCD_ShowxNum(496,348,Dust_10_Up,8,16,0x80);//10
	//下游	
	LCD_ShowxNum(600,258,Dust_03_Down,8,16,0x80);//0.3													
	LCD_ShowxNum(600,276,Dust_05_Down,8,16,0x80);//0.5
	LCD_ShowxNum(600,294,Dust_1_Down,8,16,0x80);//1.0
	LCD_ShowxNum(600,312,Dust_3_Down,8,16,0x80);//3.0	
	LCD_ShowxNum(600,330,Dust_5_Down,8,16,0x80);//5.0	
	LCD_ShowxNum(600,348,Dust_10_Down,8,16,0x80);//10
	//过滤效率
	//0.3	
	sprintf(c,"%05.2f",Efficiency_03);
	LCD_ShowString(708,258,800,16,16,(u8 *)c);
	//0.5	
	sprintf(c,"%05.2f",Efficiency_05);
	LCD_ShowString(708,276,800,16,16,(u8 *)c);
	//1		
	sprintf(c,"%05.2f",Efficiency_1);
	LCD_ShowString(708,294,800,16,16,(u8 *)c);
	//3
		
	sprintf(c,"%05.2f",Efficiency_3);
	LCD_ShowString(708,312,800,16,16,(u8 *)c);	
	//5
		
	sprintf(c,"%05.2f",Efficiency_5);
	LCD_ShowString(708,330,800,16,16,(u8 *)c);
	//10
		
	sprintf(c,"%05.2f",Efficiency_10);
	LCD_ShowString(708,348,800,16,16,(u8 *)c);
	BACK_COLOR = TU16_BackColor;
}

//指定位置显示压力值
//pressAD:AD值
void display_press(void)
{	
	u16 TU16_BackColor;
	u8 i=0;
	u16 sum=0;
	TU16_BackColor = BACK_COLOR;
	pressADvalue=Get_Adc_Average(10, 50);
	if(pressADvalue<press_zeroAD) press=0;
	else//计算出平均值
	{		
			Press_Flag++;		
			Press_DataSave[Press_Flag-1]=press_k*(pressADvalue-press_zeroAD);//计算出实际压力
			for(i=0;i<Press_Flag;i++)
				sum=sum+Press_DataSave[i];
			press=sum/Press_Flag;	
			if(Press_Flag==10)
			{
				for(i=0;i<Press_Flag;i++)	
						Press_DataSave[i]=0;
				Press_Flag=0;				
			}
	}	
	if((press>=1000||pressADvalue>=4095)&&start_testflag==1) //发现超过量程,立即关闭气门，防止损坏传感器
	{	
		//提示信息，阻力超限
		menu_pressError();	
	}		
	else 	
	{
		BACK_COLOR=LIGHTGRAY;
		LCD_ShowxNum(308,330,press,4,16,0x80);
	}	
	BACK_COLOR = TU16_BackColor;	
}
//指定位置显示流量值
void display_Flow(void)
{	
	u16 TU16_BackColor;	
	char a[5]={0};
	Sendout_uart1_Flowmeter(Flow_meter);//读取流量计的流量
	TU16_BackColor = BACK_COLOR;
	if((Flow<10||Flow>=200)&&start_testflag==1) //发现超过量程,立即关闭气门，防止损坏传感器
	{	
		if(Flow<10)	Error_Num_Flow++;
		if(Error_Num_Flow==10)
			{
				Error_Num_Flow=0;
				menu_FlowError();
			}
		//提示信息
		if(Flow>=200)	 menu_FlowError();	
	}		
	else //流量正常	
	{
		if((Flow_set-4<=Flow)&&(Flow<=Flow_set+4)) Flow=Flow_set;
		Error_Num_Flow=0;
		BACK_COLOR=LIGHTGRAY;		
		sprintf(a,"%04.1f",Flow);
		LCD_ShowString(308,284,800,16,16,(u8 *)a); 
	}	
	BACK_COLOR = TU16_BackColor;	
}



void menu_start_keytest(void)
{
	u8 t;
	u8 flag=1;
	menu_start();//开机界面
	while(flag)
	{
			t=0;
			FT5206_Scan();		
			if(ctp_dev.tpsta&TP_PRES_DOWN)		//触摸屏被按下
			{	   	
				if((ctp_dev.x[t]>350&&ctp_dev.x[t]<450)&&(ctp_dev.y[t]>200&&ctp_dev.y[t]<300))	//按下进入按钮
				{
					key_value=1;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else	key_value=0;					
		  }
			else//松开屏幕 按下确认或者返回键都退出循环
			{			
				if(key_value!=0)
				{			
					switch(key_value)
					{
						case 1:  //确认										
							flag=0;  
							key_value = 0;
							menu1();//加载初始菜单界面
							break;
										
						default:
							break;
					}
					key_value=0;//清除按键标志
				}
			}		
	}
}
//按下测试后弹出装夹试样提示
void menu_warning_keycheck(void)
{
	u8 t;
	u8 flag=1;
	u16 backup_color=0;	
	LCD_Fill(240,80,800,480,CYAN);//清掉之前的汉字
	backup_color=POINT_COLOR;
	POINT_COLOR=RED;
	if(No_Loadorclear_Flag==1)//空载测试
		Show_Str(376,202,800,24,"请按下手动按钮,合上夹具!",24,1);
	else
		Show_Str(292,202,800,24,"请放入测试样品,并按下手动按钮合上夹具!",24,1);	
	POINT_COLOR=backup_color;
	LCD_Fill(240,318,788,448,LIGHTGRAY);//大填充矩形
	Put_AnJian(407,383,114,50);	
	Show_Str(359,371,800,24,"开始测试",24,1);
	Put_AnJian(631,383,114,50);
	Show_Str(607,371,800,24,"取消",24,1);
	Air_cylinder_On;//气缸电源打开，此时按下按钮即可下压夹具。	
	while(flag)
	{
			t=0;
			FT5206_Scan();		
			if(ctp_dev.tpsta&TP_PRES_DOWN)		//触摸屏被按下
			{	   	
				if((ctp_dev.x[t]>350&&ctp_dev.x[t]<464)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408))	//按下开始测试按钮
				{
					key_value=1;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else if((ctp_dev.x[t]>574&&ctp_dev.x[t]<688)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408))	//按下取消按钮
				{
					key_value=2;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else	key_value=0;					
		  }
			else//松开屏幕 按下确认或者返回键都退出循环
			{			
				if(key_value!=0)
				{			
					switch(key_value)
					{
						case 1:  //开始测试										
							flag=0;  
							key_value = 0;
							if(No_Loadorclear_Flag==1)//空载测试
							{							
								if(model_flag==0)//NACL
									{Nacl_On;}
								else//DEHS
									{DEHS_On;}	
								MOTOR_On;//打开气泵电源	
							}
							else//开始正式测试
							{
								if(Test_num>64)	
									Data_Fullmenu(); 
								else
								{
									start_testflag=1;	
									Data_Time_down=0;
									OS_TimeMS=0;
									RTC_250MS=0;
									RTC_500MS=0;								
									RTC_12000MS=0;
									RTC_13000MS=0;
									RTC_2500MS=0;							
									MOTOR_On;//打开气泵电源	
									if(model_flag==0)//NACL
										{Nacl_On;}
									else//DEHS
										{DEHS_On;}	
									//显示清零
									Dust_03_Up=0;
									Dust_05_Up=0;
									Dust_1_Up=0;
									Dust_3_Up=0;
									Dust_5_Up=0;
									Dust_10_Up=0;
									Dust_03_Down=0;
									Dust_05_Down=0;	
									Dust_1_Down=0;
									Dust_3_Down=0;
									Dust_5_Down=0;
									Dust_10_Down=0;	
									Efficiency_03=0;
									Efficiency_05=0;
									Efficiency_1=0;
									Efficiency_3=0;
									Efficiency_5=0;
									Efficiency_10=0;	
								}
							}
							break;
						case 2:  //取消									
							flag=0;  
							key_value = 0;
							Air_cylinder_OFF;//气缸断电
							if(No_Loadorclear_Flag==1)No_Loadorclear_Flag=0;
							break;				
						default:
							break;
					}
					key_value=0;//清除按键标志
				}
			}		
	}
}

//主界面的按键功能函数
void key_function(void)
{
		u8 t=0;
		
		if(RTC_500MS&&U2DS.SedOrd ==0)
		{
			RTC_500MS=0;			
			display_time();//显示时间
			display_press();			
		}
		else if(RTC_2500MS&&U2DS.SedOrd == 0)
		{
			RTC_2500MS=0;
			display_Flow();
			display_Dust();	 			
		}		
		FT5206_Scan();	//扫描屏幕
		if(ctp_dev.tpsta&TP_PRES_DOWN)		
		{	
			if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>78)&&(ctp_dev.y[t]<128)&&start_testflag==0)//设置
				{						
					key_value=1;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
					Put_AnJian(114,183,114,50);
					Show_Str(77,167,800,32,"试验",32,1);//试验按键弹起
					AnJianPressed(114,103,114,50);
					Show_Str(77,87,800,32,"设置",32,1);//设置按键按下					
				}
			else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>238)&&(ctp_dev.y[t]<288)&&start_testflag==0)//数据
				{				
					key_value=2;//	菜单按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms					
					Put_AnJian(114,183,114,50);
					Show_Str(77,167,800,32,"试验",32,1);//试验按键弹起
					AnJianPressed(114,263,114,50);
					Show_Str(77,247,800,32,"数据",32,1);
					
				}
			else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>318)&&(ctp_dev.y[t]<368)&&start_testflag==0)//校准
				{						
					key_value=3;//	停止按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms
					Put_AnJian(114,183,114,50);
					Show_Str(77,167,800,32,"试验",32,1);//试验按键弹起
					AnJianPressed(114,343,114,50);
					Show_Str(77,327,800,32,"校准",32,1);
				}	
			else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>398)&&(ctp_dev.y[t]<448)&&start_testflag==0)//帮助
				{					
					key_value=4;//	测试按键标志					
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
					Put_AnJian(114,183,114,50);
					Show_Str(77,167,800,32,"试验",32,1);//试验按键弹起
					AnJianPressed(114,423,114,50);
					Show_Str(77,407,800,32,"帮助",32,1);
					
				}
			else if((ctp_dev.x[t]>463)&&(ctp_dev.x[t]<577)&&(ctp_dev.y[t]>398)&&(ctp_dev.y[t]<448))//启动测试
				{					
					key_value=5;//	测试按键标志					
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms					
				}
// 			else if((ctp_dev.x[t]>550)&&(ctp_dev.x[t]<800)&&(ctp_dev.y[t]>0)&&(ctp_dev.y[t]<55)&&Test_num<65)//上游清零
// 				{					
// 					key_value=7;//	测试按键标志					
// 					Loosen_Lcd();  //等待按键松开
// 					speekers(100);//蜂鸣器响100ms					
// 				}		
// 			else if((ctp_dev.x[t]>350)&&(ctp_dev.x[t]<464)&&(ctp_dev.y[t]>398)&&(ctp_dev.y[t]<448)&&Test_num<65)//下游清零
// 				{					
// 					key_value=6;//	测试按键标志					
// 					Loosen_Lcd();  //等待按键松开
// 					speekers(100);//蜂鸣器响100ms					
// 				}	
			else if((ctp_dev.x[t]>367)&&(ctp_dev.x[t]<410)&&(ctp_dev.y[t]>158)&&(ctp_dev.y[t]<208)&&start_testflag==0)//流量选择32L/MIN
				{								
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
					if(flow_flag==1||flow_flag==2) 
					{
						flow_flag=0;
						Flow_set=32;
						GUI_SetColor(GREEN,LIGHTGRAY);//设置前景色和背景色
						GUI_LoadPic(362,161,(u8 *)gImage_on,43,43) ;
						GUI_SetColor(RED,LIGHTGRAY);//设置前景色和背景色
						GUI_LoadPic(509,161,(u8 *)gImage_off,43,43) ;
						GUI_LoadPic(656,161,(u8 *)gImage_off,43,43) ;
						
						LCD_Fill(250,178,310,198,LGRAY);//小填充矩形						
						Show_Str(260,180,800,16,"32.0",16,1);
					}				
				}	
			else if((ctp_dev.x[t]>509)&&(ctp_dev.x[t]<552)&&(ctp_dev.y[t]>158)&&(ctp_dev.y[t]<208)&&start_testflag==0)//流量选择85L/MIN
				{
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
					if(flow_flag==0||flow_flag==2)
					{
						flow_flag=1;
						Flow_set=85;
						GUI_SetColor(RED,LIGHTGRAY);//设置前景色和背景色
						GUI_LoadPic(362,161,(u8 *)gImage_off,43,43) ;
						GUI_LoadPic(656,161,(u8 *)gImage_off,43,43) ;
						GUI_SetColor(GREEN,LIGHTGRAY);//设置前景色和背景色
						GUI_LoadPic(509,161,(u8 *)gImage_on,43,43) ;
						
						LCD_Fill(250,178,310,198,LGRAY);//小填充矩形
						Show_Str(260,180,800,16,"85.0",16,1);//test	
					}
				}
			else if((ctp_dev.x[t]>656)&&(ctp_dev.x[t]<699)&&(ctp_dev.y[t]>158)&&(ctp_dev.y[t]<208)&&start_testflag==0)//流量选择95L/MIN
				{
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
					if(flow_flag==0||flow_flag==1)
					{
						flow_flag=2;
						Flow_set=95;
						GUI_SetColor(RED,LIGHTGRAY);//设置前景色和背景色
						GUI_LoadPic(362,161,(u8 *)gImage_off,43,43) ;
						GUI_LoadPic(509,161,(u8 *)gImage_off,43,43) ;
						GUI_SetColor(GREEN,LIGHTGRAY);//设置前景色和背景色
						GUI_LoadPic(656,161,(u8 *)gImage_on,43,43) ;
						
						LCD_Fill(250,178,310,198,LGRAY);//小填充矩形
						Show_Str(260,180,800,16,"95.0",16,1);//test	
					}
				}		
			else//按的是无效区域				
					key_value=0;				
		}
		else
		{
			if(key_value!=0)
				{			
					switch(key_value)
					{
						case 1:  //设置
							key_value = 0;																		
							menu_setting();
							menu_setting_key();
							break;
						case 2://数据
							key_value = 0;
							menu_datacheck();
							menu_datacheck_key();
							break;											
						case 3://校准
							key_value = 0;
							menu_password();							
							break;
						case 4://帮助	
							key_value = 0;								
							menu_help();						
							break;
						case 5://启动测试	
							key_value = 0;
							if(start_testflag==0)//启动测试
							{														
								menu_warning_keycheck();//提示信息													
							}
							else	Stop_Flag=1;
							menu1();//加载初始菜单界面
							break;
// 							case 6:
// 								key_value = 0;
// 								//读取压差传感器的零点AD
// 								press_zeroAD=Get_Adc_Average(10, 50);
// 								//读取一下下游的计数器的零点值
// 								Zero_Eff_flag=1;								
// 								OS_TimeMS=0;
// 								RTC_250MS=0;
// 								RTC_500MS=0;
// 								RTC_1000MS=0;
// 								RTC_1250MS=0;
// 								RTC_1500MS=0;	
// 								menu1();//加载初始菜单界面		
// 								break;	
// 							case 7:
// 								key_value = 0;							
// 								//读取下游的计数器的零点值
// 								Zero_Eff_flag=2;								
// 								OS_TimeMS=0;
// 								RTC_250MS=0;
// 								RTC_500MS=0;
// 								RTC_1000MS=0;
// 								RTC_1250MS=0;
// 								RTC_1500MS=0;	
// 								menu1();//加载初始菜单界面		
// 								break;	
						default:
							break;						
					}				
					key_value=0;//清除按键标志
				}
		}
}
/******************************************************************/
//函数功能： 用于新进入一个界面时，初始化显示日期、时间
//对于1s钟中断一次显示秒，其它时间、日期没有变化时不再更新
//节省中断处理函数时间
/*******************************************************************/
void display_time(void)
{
	u16 TU16_BackColor;
	u16 TU16_PointColor;

	TU16_BackColor = BACK_COLOR;		
	TU16_PointColor = POINT_COLOR;
	POINT_COLOR = BLACK;
	BACK_COLOR = CYAN;
	LCD_ShowNum(60,0,calendar.w_year,4,24);
	Show_Str(108,0,lcddev.width,24,"-",24,1);									  
	LCD_ShowxNum(120,0,calendar.w_month,2,24,0X80);
	Show_Str(144,0,lcddev.width,24,"-",24,1);									  
	LCD_ShowxNum(156,0,calendar.w_date,2,24,0x80); 

	LCD_ShowxNum(72,24,calendar.hour,2,24,0X80);
	Show_Str(96,24,lcddev.width,24,":",24,1);									  
	LCD_ShowxNum(108,24,calendar.min,2,24,0X80);
	Show_Str(132,24,lcddev.width,24,":",24,1);									  
	LCD_ShowxNum(144,24,calendar.sec,2,24,0X80);	
	POINT_COLOR = TU16_PointColor; 
	BACK_COLOR = TU16_BackColor;		
}

void speekers(u16 time)
{
 	GPIO_SetBits(GPIOB,GPIO_Pin_8);						 //PB8 输出高
 	delay_ms(time);//更新显示界面
 	GPIO_ResetBits(GPIOB,GPIO_Pin_8);					//PB8 输出低 
}
//冒泡法排序 从小到大排列
void bubble_sort(float a[], u8 n)
{	u8 i, j;
	float temp;
	for (j = 0; j < n - 1; j++)
			for (i = 0; i < n - 1 - j; i++)
			 if(a[i] > a[i + 1])
					{temp=a[i]; a[i]=a[i+1]; a[i+1]=temp;}
}
//数据满提示
void Data_Fullmenu(void)
{
	u16 backup_color=0;	
	u8 t;
	u8 flag=1;
	LCD_Fill(240,80,800,480,CYAN);//清掉之前的汉字
	backup_color=POINT_COLOR;
	POINT_COLOR=RED;
	Show_Str(316,202,800,24,"数据已满,请进入<数据>菜单,进行清空!",24,1);
	POINT_COLOR=backup_color;
	LCD_Fill(240,318,788,448,LIGHTGRAY);//大填充矩形	
	Put_AnJian(520,383,114,50);
	Show_Str(496,371,800,24,"确定",24,1);	
	while(flag)
	{
			t=0;
			FT5206_Scan();		
			if(ctp_dev.tpsta&TP_PRES_DOWN)		//触摸屏被按下
			{	   	
				if((ctp_dev.x[t]>463&&ctp_dev.x[t]<577)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408))	//按下确定按钮
				{
					key_value=1;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}				
				else	key_value=0;					
		  }
			else//松开屏幕 按下确认或者返回键都退出循环
			{			
				if(key_value!=0)
				{			
					switch(key_value)
					{
						case 1:  //开始测试										
							flag=0;  
							key_value = 0;							
							menu1();//加载初始菜单界面							
							break;
								
						default:
							break;
					}
					key_value=0;//清除按键标志
				}
			}		
	}
}
//清空警告
void Erase_Waring(void)
{
	u16 backup_color=0;	
	u8 t;
	u8 flag=1;
	LCD_Fill(240,80,800,480,CYAN);//清掉之前的汉字
	backup_color=POINT_COLOR;
	POINT_COLOR=RED;
	Show_Str(280,202,800,24,"即将清除所有数据,请确定是否继续进行!",24,1);
	POINT_COLOR=backup_color;
	LCD_Fill(240,318,788,448,LIGHTGRAY);//大填充矩形	
	Put_AnJian(407,383,114,50);
	Show_Str(383,371,800,24,"确定",24,1);	
	Put_AnJian(631,383,114,50);
	Show_Str(607,371,800,24,"取消",24,1);
	while(flag)
	{
			t=0;
			FT5206_Scan();		
			if(ctp_dev.tpsta&TP_PRES_DOWN)		//触摸屏被按下
			{	   	
				if((ctp_dev.x[t]>350&&ctp_dev.x[t]<464)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408))	//按下确定按钮
				{
					key_value=1;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else if((ctp_dev.x[t]>574&&ctp_dev.x[t]<688)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408))	//按下取消按钮
				{
					key_value=2;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}	
				else	key_value=0;					
		  }
			else//松开屏幕 按下确认或者返回键都退出循环
			{			
				if(key_value!=0)
				{			
					switch(key_value)
					{
						case 1:  									
							flag=0;  
							key_value = 0;
							SPI_Flash_Erase_Sector(0);
							delay_ms(100);	
							SPI_Flash_Erase_Sector(1);
							delay_ms(100);
							SPI_Flash_Erase_Sector(2);
							delay_ms(100);
							SPI_Flash_Erase_Sector(3);
							delay_ms(100);
							Data_checkNum=1;
							Test_num=1;
							AT24CXX_WriteOneByte(SAVE_ADDR_Num,Test_num);
							break;
						case 2:  									
							flag=0;  
							key_value = 0;												
							break;		
						default:
							break;
					}
					key_value=0;//清除按键标志
				}
			}		
	}
}
void menu_pressError(void)//测试阻力超限提示界面
{
	u16 backup_color=0;	
	u8 t;
	u8 flag=1;
	LCD_Fill(240,80,800,480,CYAN);//清掉之前的汉字
	backup_color=POINT_COLOR;
	POINT_COLOR=RED;
	Show_Str(340,202,800,24,"测试阻力超限,请检查相关传感器!",24,1);
	POINT_COLOR=backup_color;
	LCD_Fill(240,318,788,448,LIGHTGRAY);//大填充矩形	
	Put_AnJian(520,383,114,50);
	Show_Str(496,371,800,24,"确定",24,1);
	Error_Flag=1;	
	while(Error_Flag&&start_testflag)//出现异常立即关闭
	{
			if(CLJ_close<2)
			{	
				if(RTC_250MS&&CLJ_close==0)
					{
						RTC_250MS=0;	
						U2DS.SedOrd = 0	;
						Sendout_uart1_CLJ260(CLJ_R260_UpAddr,0x03);		//关闭上游计数器开始测量
					}
				else if(RTC_250MS&&CLJ_close==1)
					{
						RTC_250MS=0;
						U2DS.SedOrd = 0	;
						Sendout_uart1_CLJ260(CLJ_R260_DownAddr,0x03);	//关闭下游计数器开始测量
					}
				}
				else 
				{
					Data_Time_down=0;
					Stop_Flag=0;
					Error_Flag=0;
					start_testflag=0;
					Test_over=0;
					CLJ_START=0;
					CLJ_close=0;					
					MOTOR_OFF;//关闭气泵电源
					Air_cylinder_OFF;//气缸断电，夹具升起来
					if(model_flag==0)//NACL
					{Nacl_OFF;}
					else//DEHS
					{DEHS_OFF;}	
										
				}				
				USART_Com();        //;串口通讯		
				ZD_Port();					//;终端端口通讯 				
	}
	while(flag)
	{
			t=0;
			FT5206_Scan();		
			if(ctp_dev.tpsta&TP_PRES_DOWN)		//触摸屏被按下
			{	   	
				if((ctp_dev.x[t]>463&&ctp_dev.x[t]<577)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408))	//按下确定按钮
				{
					key_value=1;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}				
				else	key_value=0;					
		  }
			else//松开屏幕 按下确认或者返回键都退出循环
			{			
				if(key_value!=0)
				{			
					switch(key_value)
					{
						case 1:  //开始测试										
							flag=0;  
							key_value = 0;							
							menu1();//加载初始菜单界面							
							break;
								
						default:
							break;
					}
					key_value=0;//清除按键标志
				}
			}		
	}	
}
void menu_FlowError(void)//测试流量超限提示界面
{
	u16 backup_color=0;	
	u8 t;
	u8 flag=1;
	LCD_Fill(240,80,800,480,CYAN);//清掉之前的汉字
	backup_color=POINT_COLOR;
	POINT_COLOR=RED;
	Show_Str(340,202,800,24,"测试流量超限,请检查相关传感器!",24,1);
	POINT_COLOR=backup_color;
	LCD_Fill(240,318,788,448,LIGHTGRAY);//大填充矩形	
	Put_AnJian(520,383,114,50);
	Show_Str(496,371,800,24,"确定",24,1);
	Error_Flag=1;	
	while(Error_Flag&&start_testflag)//出现异常立即关闭
	{			
				if(CLJ_close<2)
				{	
					if(RTC_250MS&&CLJ_close==0)
					{
						RTC_250MS=0;					
						Sendout_uart1_CLJ260(CLJ_R260_UpAddr,0x03);		//关闭上游计数器开始测量
					}
					else if(RTC_250MS&&CLJ_close==1)
					{
						RTC_250MS=0;
						Sendout_uart1_CLJ260(CLJ_R260_DownAddr,0x03);	//关闭下游计数器开始测量
					}
				}
				else
				{										
					Stop_Flag=0;
					Error_Flag=0;
					Data_Time_down=0;
					start_testflag=0;
					Test_over=0;
					CLJ_START=0;
					CLJ_close=0;					
					MOTOR_OFF;//关闭气泵电源
					Air_cylinder_OFF;//气缸断电，夹具升起来
					if(model_flag==0)//NACL
					{Nacl_OFF;}
					else//DEHS
					{DEHS_OFF;}	
										
				}
				USART_Com();        //;串口通讯		
				ZD_Port();					//;终端端口通讯 				
	}
	while(flag)
	{
			t=0;
			FT5206_Scan();		
			if(ctp_dev.tpsta&TP_PRES_DOWN)		//触摸屏被按下
			{	   	
				if((ctp_dev.x[t]>463&&ctp_dev.x[t]<577)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408))	//按下确定按钮
				{
					key_value=1;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}				
				else	key_value=0;					
		  }
			else//松开屏幕 按下确认或者返回键都退出循环
			{			
				if(key_value!=0)
				{			
					switch(key_value)
					{
						case 1:  //开始测试										
							flag=0;  
							key_value = 0;							
							menu1();//加载初始菜单界面							
							break;
								
						default:
							break;
					}
					key_value=0;//清除按键标志
				}
			}		
	}	
}
//开机界面
void menu_start(void)
{
	Show_Str(144,50,lcddev.width,32,"GL3150颗粒物过滤效率及气阻测试仪",32,1);		
	Show_Str(226,100,800,24,"Mask Filter Efficiency Tester",24,1);
	Show_Str(272,154,lcddev.width,32,"进入请按下方logo",32,1);
	LCD_DrawRectangle(270, 152, 530, 188);	
	GUI_SetColor(RED,CYAN);//设置前景色和背景色	
 	GUI_LoadPic(350,200, (u8 *)gImage_gl_logo, 100, 100);	// 显示100*100的图标 
 	Show_Str(280,320,lcddev.width,24,"武汉国量仪器有限公司",24,1);
 	Show_Str(304,360,lcddev.width,24,"TEL:027-87467581",24,1);
 	Show_Str(316,400,lcddev.width,24,"WWW.WHGLYQ.COM",24,1);
}
//测试界面
void menu1(void)
{
	u16 backup_color=0;
	u16	color=0;
	LCD_Clear(CYAN);
	Put_AnJian(114,103,114,50);	
	AnJianPressed(114,183,114,50);//默认进入的是试验界面
	Put_AnJian(114,263,114,50);
	Put_AnJian(114,343,114,50);
	Put_AnJian(114,423,114,50);
	//画一条粗直线
	gui_draw_vline(228,0,479,RED);
	//插入汉字
	POINT_COLOR = BLACK;
	Show_Str(77,87,800,32,"设置",32,1);
	Show_Str(77,167,800,32,"试验",32,1);
	Show_Str(77,247,800,32,"数据",32,1);
	Show_Str(77,327,800,32,"校准",32,1);
	Show_Str(77,407,800,32,"帮助",32,1);
	Show_Str(240,0,800,24,"颗粒物过滤效率及气阻测试仪",24,1);
	Show_Str(492,25,lcddev.width,16,"v1.0",16,1);	
	gui_draw_hline(240,24,290+24,RED);
	Show_Str(240,25,800,16,"Mask Filter Efficiency Tester",16,1);
	
	//插入公司图标
	GUI_SetColor(RED,CYAN);//设置前景色和背景色
	GUI_LoadPic(550,0,(u8 *)gImage_logo,245,55) ;
	//试样编号
	Show_Str(240,87,800,24,"试样编号",24,1);
	color=BACK_COLOR;
	BACK_COLOR=CYAN;
	LCD_ShowxNum(360,87,Test_num,2,24,0X80);
	BACK_COLOR=color;
	//图形填充
	LCD_Fill(240,158,788,208,LIGHTGRAY);//大填充矩形
	LCD_Fill(250,148,370,168,LGRAY);//小填充矩形
	Show_Str(278,150,800,16,"流量设定",16,1);
	//流量显示
	LCD_Fill(250,178,310,198,LGRAY);//小填充矩形	
	Show_Str(320,180,800,16,"L/min",16,1);
	//显示选择的流量
	if(flow_flag==0)//默认选择的是32L/MIN
	{		
		GUI_SetColor(GREEN,LIGHTGRAY);//设置前景色和背景色
		GUI_LoadPic(362,161,(u8 *)gImage_on,43,43) ;
		GUI_SetColor(RED,LIGHTGRAY);//设置前景色和背景色
		GUI_LoadPic(509,161,(u8 *)gImage_off,43,43) ;
		GUI_LoadPic(656,161,(u8 *)gImage_off,43,43) ;
		Show_Str(260,180,800,16,"32.0",16,1);
	}
	else if(flow_flag==1)//选择85L/MIN
	{
		GUI_SetColor(RED,LIGHTGRAY);//设置前景色和背景色
		GUI_LoadPic(362,161,(u8 *)gImage_off,43,43) ;
		GUI_LoadPic(656,161,(u8 *)gImage_off,43,43) ;
		GUI_SetColor(GREEN,LIGHTGRAY);//设置前景色和背景色
		GUI_LoadPic(509,161,(u8 *)gImage_on,43,43) ;
		Show_Str(260,180,800,16,"85.0",16,1);	
	}	
	else if(flow_flag==2)//选择95L/MIN
	{
		GUI_SetColor(RED,LIGHTGRAY);//设置前景色和背景色
		GUI_LoadPic(362,161,(u8 *)gImage_off,43,43) ;
		GUI_LoadPic(509,161,(u8 *)gImage_off,43,43) ;
		GUI_SetColor(GREEN,LIGHTGRAY);//设置前景色和背景色
		GUI_LoadPic(656,161,(u8 *)gImage_on,43,43) ;
		Show_Str(260,180,800,16,"95.0",16,1);
	}
	Show_Str(405,171,800,24,"32L/min",24,1);
	Show_Str(552,171,800,24,"85L/min",24,1);
	Show_Str(699,171,800,24,"95L/min",24,1);
	//监控区
	LCD_Fill(240,238,788,368,LIGHTGRAY);//大填充矩形
	LCD_Fill(250,228,370,248,LGRAY);//小填充矩形
	Show_Str(278,230,800,16,"数据监控",16,1);
	//显示文字
	Show_Str(504,240,800,16,"上游",16,1);
	Show_Str(608,240,800,16,"下游",16,1);
	Show_Str(712,240,800,16,"效率",16,1);
	Show_Str(744,240,800,16,"%",16,1);
	Show_Str(400,258,800,16,"0.3um:",16,1);
	Show_Str(400,276,800,16,"0.5um:",16,1);
	Show_Str(400,294,800,16,"1.0um:",16,1);
	Show_Str(400,312,800,16,"3.0um:",16,1);
	Show_Str(400,330,800,16,"5.0um:",16,1);
	Show_Str(400,348,800,16," 10um:",16,1);
	Show_Str(268,284,800,16,"流量      L/min",16,1);//test
	Show_Str(268,330,800,16,"压差      Pa",16,1);//test
	//Show_Str(268,330,800,16,"静电中和  关",16,1);//test
	//启动、停止键	
	Put_AnJian(520,423,114,50);
	if(start_testflag==1)
		{
			backup_color=POINT_COLOR;
			POINT_COLOR=RED;
			Show_Str(488,407,800,32,"停止",32,1);
			POINT_COLOR=backup_color;
		}
	else		
	{
		Show_Str(488,407,800,32,"测试",32,1);
	}
}

void menu_setting(void)//设置界面
{
	LCD_Fill(240,80,800,480,CYAN);//清掉之前的汉字
	//试样编号	
	Show_Str(240,87,800,24,"操作员号",24,1); 
	LCD_Fill(360,87,500,114,LIGHTGRAY);
	//日期时间设置
	Show_Str(240,133,800,24,"日期设置",24,1); 
	LCD_Fill(360,133,500,163,LIGHTGRAY);
	Show_Str(240,179,800,24,"时间设置",24,1); 
	LCD_Fill(360,179,500,209,LIGHTGRAY);
	//图形填充
	LCD_Fill(240,238,788,288,LIGHTGRAY);//大填充矩形
	LCD_Fill(250,228,370,248,LGRAY);//小填充矩形
	Show_Str(278,230,800,16,"模式选择",16,1);	
	
	if(model_flag==0)//默认为盐性
	{
		GUI_SetColor(GREEN,LIGHTGRAY);//设置前景色和背景色
		GUI_LoadPic(400,241,(u8 *)gImage_on,43,43) ;
		GUI_SetColor(RED,LIGHTGRAY);//设置前景色和背景色
		GUI_LoadPic(600,241,(u8 *)gImage_off,43,43) ;
	}
	
	else if(model_flag==1)//选择油性
	{
		GUI_SetColor(RED,LIGHTGRAY);//设置前景色和背景色
		GUI_LoadPic(400,241,(u8 *)gImage_off,43,43) ;
		GUI_SetColor(GREEN,LIGHTGRAY);//设置前景色和背景色
		GUI_LoadPic(600,241,(u8 *)gImage_on,43,43) ;	
	}
	Show_Str(445,251,800,24,"NACL",24,1);
	Show_Str(645,251,800,24,"DEHS",24,1);
	//监控区
	LCD_Fill(240,318,788,448,LIGHTGRAY);//大填充矩形
	LCD_Fill(250,308,370,328,LGRAY);//小填充矩形
	Show_Str(270,310,800,16,"空载/装夹",16,1);
	//显示文字
	if(No_Loadorclear_Flag==0)
	{
		Put_AnJian(358,383,114,50);	
		Show_Str(310,371,800,24,"空载试验",24,1);	
	}
	else if(No_Loadorclear_Flag==1)
	{
		AnJianPressed(358,383,114,50);	
		Show_Str(334,371,800,24,"停止",24,1);	
	}	
	Put_AnJian(533,383,114,50);	
	Show_Str(509,371,800,24,"清洁",24,1);
						
	Put_AnJian(708,383,114,50);
	Show_Str(684,371,800,24,"装夹",24,1);

	//显示编号日期时间
	BACK_COLOR=LIGHTGRAY;
	LCD_ShowxNum(394,87,number,4,24,0x80);//编号
	LCD_ShowString(370,133,800,24,24,"    -  -  ");//日期
	LCD_ShowxNum(370,133,calendar.w_year,4,24,0x80);							
	LCD_ShowxNum(430,133,calendar.w_month,2,24,0x80);							
	LCD_ShowxNum(466,133,calendar.w_date,2,24,0x80);	
	LCD_ShowString(382,179,800,24,24,"  :  :  ");//时间
	LCD_ShowxNum(382,179,calendar.hour,2,24,0x80);							
	LCD_ShowxNum(418,179,calendar.min,2,24,0x80);							
	LCD_ShowxNum(454,179,calendar.sec,2,24,0x80);	
}
//空载或清洁试验测试界面
void No_LoadTest_orclearMenu(void)
{
	u8 flag;
	u8 t;
	u8 Test_num=0;
	u8 Stop_flag=0;	
	LCD_Fill(548,86,752,210,LIGHTGRAY);//大填充矩形
	LCD_DrawRectangle(550,88,750,208);//画表格外框
	//画水平线
	gui_draw_hline(550,108,200,BLACK);
	gui_draw_hline(550,128,200,BLACK);
	gui_draw_hline(550,148,200,BLACK);
	gui_draw_hline(550,168,200,BLACK);
	gui_draw_hline(550,188,200,BLACK);
	//画一条垂直线
	gui_draw_vline(650,88,120,BLACK);
	//显示文字
	Show_Str(584,90,800,16,"时间",16,1);
	Show_Str(684,90,800,16,"浓度",16,1);	
	flag=1;
	OS_TimeMS=0;
	RTC_250MS=0;
	RTC_500MS=0;
	RTC_12000MS=0;
	RTC_13000MS=0;
	RTC_2500MS=0;
	while(flag)
	{
			t=0;		
			USART_Com();        //;串口通讯
			ZD_Port();					//;终端端口通讯 
			DataProcess();			//;终端数据处理及数据发送	
			if(RTC_250MS&&Stop_flag==1&&CLJ_close==0)
			{
				RTC_250MS=0;			
				Sendout_uart1_CLJ260(CLJ_R260_DownAddr,0x03);	//停止下游计数器				
				continue;				
			}
			else if(Stop_flag==1&&RTC_250MS&&CLJ_close==2)
			{						
				RTC_250MS=0;				
				CLJ_close=0;
				CLJ_START=0;	
				flag=0;
				Stop_flag=0;
				Test_over=0;
				Data_Time_down=0;
				continue;
			}
			if(RTC_250MS&&CLJ_START==0)//启动下游计数器
			{		
				RTC_250MS=0;
				Sendout_uart1_CLJ260(CLJ_R260_DownAddr,0x02);	//启动下游计数器
			}	
			if(RTC_12000MS&&CLJ_START==2)//每10S读取1次下游计数器的值和流量的值
			{
				RTC_12000MS=0;
				Sendout_uart1_CLJ260(CLJ_R260_DownAddr,0x01);//读取下游的离子浓度	
				//显示时间	
				LCD_ShowxNum(568,110+(Test_num)*20,calendar.hour,2,16,0X80);
				Show_Str(584,110+(Test_num)*20,lcddev.width,16,":",16,1);									  
				LCD_ShowxNum(592,110+(Test_num)*20,calendar.min,2,16,0X80);
				Show_Str(608,110+(Test_num)*20,lcddev.width,16,":",16,1);									  
				LCD_ShowxNum(616,110+(Test_num)*20,calendar.sec,2,16,0X80);	
				//显示下游浓度数
				LCD_ShowxNum(668,110+(Test_num)*20,Dust_03_Down,8,16,0x80);//0.3								
				Test_num++;	
			}			
			if(Test_num==5)
			{
					Test_num=0;
					LCD_Fill(548,86,752,210,LIGHTGRAY);//大填充矩形
					LCD_DrawRectangle(550,88,750,208);//画表格外框
					//画水平线
					gui_draw_hline(550,108,200,BLACK);
					gui_draw_hline(550,128,200,BLACK);
					gui_draw_hline(550,148,200,BLACK);
					gui_draw_hline(550,168,200,BLACK);
					gui_draw_hline(550,188,200,BLACK);
					//画一条垂直线
					gui_draw_vline(650,88,120,BLACK);
					//显示文字
					Show_Str(584,90,800,16,"时间",16,1);
					Show_Str(684,90,800,16,"浓度",16,1);
			}
			FT5206_Scan();	
			if(ctp_dev.tpsta&TP_PRES_DOWN)		//触摸屏被按下
			{	   	
				if((ctp_dev.x[t]>301&&ctp_dev.x[t]<415)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408)&&No_Loadorclear_Flag==1)	//按下空载停止按钮
				{
					key_value=1;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms						
					Put_AnJian(358,383,114,50);	
					Show_Str(310,371,800,24,"空载试验",24,1);	
				}					
				else if((ctp_dev.x[t]>476&&ctp_dev.x[t]<590)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408)&&No_Loadorclear_Flag==2)	//按下清洁停止按钮
				{
					key_value=1;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms						
					Put_AnJian(533,383,114,50);	
					Show_Str(509,371,800,24,"清洁",24,1);					
				}				
				else	key_value=0;					
		  }
			else//松开屏幕 按下确认或者返回键都退出循环
			{			
				if(key_value!=0)
				{			
					switch(key_value)
					{
						case 1: 					
							Stop_flag=1;
							key_value = 0;
							if(No_Loadorclear_Flag==1)//停止
							{								
								if(model_flag==0)//NACL
								{Nacl_OFF;}
								else//DEHS
								{DEHS_OFF;}	
								Air_cylinder_OFF;//气缸断电
								MOTOR_OFF;
							}							
							else //清洁
								MOTOR_OFF;
							LCD_Fill(548,86,752,210,CYAN);//大填充矩形
							No_Loadorclear_Flag=0;
							break;
								
						default:
							break;
					}
					key_value=0;//清除按键标志
				}
			}
	}
}
//设置界面的装夹试样提示
void menu_warning_press(void)
{
	u8 t;
	u8 flag=1;
	u16 backup_color=0;	
	LCD_Fill(240,80,800,480,CYAN);//清掉之前的汉字
	backup_color=POINT_COLOR;
	POINT_COLOR=RED;
	Show_Str(292,202,800,24,"请放入测试样品,并按下手动按钮合上夹具!",24,1);	
	POINT_COLOR=backup_color;
	LCD_Fill(240,318,788,448,LIGHTGRAY);//大填充矩形	
	Put_AnJian(520,383,114,50);
	Show_Str(488,371,800,24,"取消",24,1);
	while(flag)
	{
			t=0;
			FT5206_Scan();		
			if(ctp_dev.tpsta&TP_PRES_DOWN)		//触摸屏被按下
			{	   	
				if((ctp_dev.x[t]>463&&ctp_dev.x[t]<577)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408))	//按下取消按钮
				{
					key_value=1;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else	key_value=0;					
		  }
			else//松开屏幕 按下确认或者返回键都退出循环
			{			
				if(key_value!=0)
				{			
					switch(key_value)
					{						
						case 1:  //取消									
							flag=0;  
							key_value = 0;
							Air_cylinder_OFF;//气缸断电	
							menu_setting();								
							break;				
						default:
							break;
					}
					key_value=0;//清除按键标志
				}
			}		
	}
}
//设置界面的按键设置
void menu_setting_key(void)
{
	u8 t;
	u8 flag=1;	
	while(flag)
	{
			t=0;
			if(RTC_500MS)
			{
				RTC_500MS=0;			
				display_time();//显示时间				
			}	
			FT5206_Scan();		
			if(ctp_dev.tpsta&TP_PRES_DOWN)		//触摸屏被按下
			{	   	
				if((ctp_dev.x[t]>360&&ctp_dev.x[t]<460)&&(ctp_dev.y[t]>87&&ctp_dev.y[t]<114))	//操作员号
				{
					key_value=1;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else if((ctp_dev.x[t]>360&&ctp_dev.x[t]<460)&&(ctp_dev.y[t]>133&&ctp_dev.y[t]<163))	//日期设置
				{
					key_value=2;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else if((ctp_dev.x[t]>360&&ctp_dev.x[t]<460)&&(ctp_dev.y[t]>179&&ctp_dev.y[t]<209))	//时间设置
				{
					key_value=3;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else if((ctp_dev.x[t]>400&&ctp_dev.x[t]<460)&&(ctp_dev.y[t]>240&&ctp_dev.y[t]<280))	//选择nacl
				{
					key_value=4;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else if((ctp_dev.x[t]>600&&ctp_dev.x[t]<660)&&(ctp_dev.y[t]>240&&ctp_dev.y[t]<280))	//选择DEHS
				{
					key_value=5;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else if((ctp_dev.x[t]>301&&ctp_dev.x[t]<415)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408))	//选择空载试验
				{
					key_value=6;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else if((ctp_dev.x[t]>476&&ctp_dev.x[t]<590)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408))	//选择清洁
				{
					key_value=12;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms							
				}
				else if((ctp_dev.x[t]>651&&ctp_dev.x[t]<765)&&(ctp_dev.y[t]>358&&ctp_dev.y[t]<408))	//选择装夹
				{
					key_value=7;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms					
				}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>158)&&(ctp_dev.y[t]<208))//试验
					{				
						key_value=8;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						Put_AnJian(114,103,114,50);
						Show_Str(77,87,800,32,"设置",32,1);//设置按键弹起
						AnJianPressed(114,183,114,50);
						Show_Str(77,167,800,32,"试验",32,1);						
					}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>238)&&(ctp_dev.y[t]<288))//数据
					{				
						key_value=9;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						Put_AnJian(114,103,114,50);
						Show_Str(77,87,800,32,"设置",32,1);//设置按键弹起
						AnJianPressed(114,263,114,50);
						Show_Str(77,247,800,32,"数据",32,1);						
					}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>318)&&(ctp_dev.y[t]<368))//校准
					{						
						key_value=10;//	停止按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms
						Put_AnJian(114,103,114,50);
						Show_Str(77,87,800,32,"设置",32,1);//设置按键弹起
						AnJianPressed(114,343,114,50);
						Show_Str(77,327,800,32,"校准",32,1);
					}	
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>398)&&(ctp_dev.y[t]<448))//帮助
					{					
						key_value=11;//	测试按键标志					
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms	
						Put_AnJian(114,103,114,50);
						Show_Str(77,87,800,32,"设置",32,1);//设置按键弹起
						AnJianPressed(114,423,114,50);
						Show_Str(77,407,800,32,"帮助",32,1);
						
					}
				else	key_value=0;					
		  }
			else//松开屏幕 按下确认或者返回键都退出循环
			{			
				if(key_value!=0)
				{			
					switch(key_value)
					{
						case 1:  //编号设置			
							key_value = 0;
							set_numkey=1;
							menu_setkeyboard();
							menu_setting();
							set_numkey=0;
							break;
						case 2://日期设置
							key_value=0;
							set_numkey=2;
							menu_setkeyboard();
							menu_setting();
							set_numkey=0;
							break;	
						case 3://时间设置
							key_value=0;
							set_numkey=3;
						  menu_setkeyboard();
							menu_setting();
							set_numkey=0;
							break;
						case 4://NACL
							key_value=0;
							model_flag=0;							
							GUI_SetColor(GREEN,LIGHTGRAY);//设置前景色和背景色
							GUI_LoadPic(400,241,(u8 *)gImage_on,43,43) ;
							GUI_SetColor(RED,LIGHTGRAY);//设置前景色和背景色
							GUI_LoadPic(600,241,(u8 *)gImage_off,43,43) ;											
							break;
						case 5://DEHS
							key_value=0;
							model_flag=1;
							GUI_SetColor(RED,LIGHTGRAY);//设置前景色和背景色
							GUI_LoadPic(400,241,(u8 *)gImage_off,43,43) ;
							GUI_SetColor(GREEN,LIGHTGRAY);//设置前景色和背景色
							GUI_LoadPic(600,241,(u8 *)gImage_on,43,43) ;							
							break;
						case 6://空载试验
							key_value=0;
							No_Loadorclear_Flag=1;
							menu_warning_keycheck();							
							//空载试验界面							
							if(No_Loadorclear_Flag)	
								{
									menu_setting();
									No_LoadTest_orclearMenu();							
								}
							menu_setting();							
							break;
						case 7://装夹
							key_value=0;
							Air_cylinder_On;							
							menu_warning_press();
							break;
						case 8://试验
							key_value=0;
							menu1();
							flag=0;
							break;	
						case 9://数据
							key_value=0;
							menu_datacheck();
							menu_datacheck_key();
							flag=0;
							break;
						case 10://校准
							key_value=0;
							menu_password();
							flag=0;
							break;
						case 11://帮助
							key_value=0;							
							menu_help();
							flag=0;//退出本循环
							break;
						case 12://清洁测试腔体
							key_value=0;
							//清洁腔体界面
							AnJianPressed(533,383,114,50);	
							Show_Str(509,371,800,24,"停止",24,1);
							No_Loadorclear_Flag=2;
							MOTOR_On;//打开气泵
							No_LoadTest_orclearMenu();							
							menu_setting();							
							break;
						default:
							break;
					}
					key_value=0;//清除按键标志
				}
			}		
	}
}
//显示虚拟键盘
void JianPan_Displayfu(void)
{
	Put_AnJian(520,240,440,280);//键盘大边框
	LCD_Fill(310,113,730,153,BLUE);
	POINT_COLOR = WHITE;
	Put_AnJian(710, 133, 40, 40);//"x"
	lcd_draw_bline(700, 123, 720, 143, 2, RED);
	lcd_draw_bline(700, 143, 720, 123, 2, RED);
	AnJianPressed(410, 186, 200, 40);//	放置输入窗口
	
	if(set_numkey==1)
	{
		Show_Str(320,121,800,24,"操作员号",24,1);
		Show_Str(428,125,800,16,"范围:(1-9999)",16,1);
	}	
	else if(set_numkey==2)
	{
		Show_Str(320,121,800,24,"日期设置",24,1);
		Show_Str(428,125,800,16,"例如:20200507",16,1);
	}
	else if(set_numkey==3)
	{
		Show_Str(320,121,800,24,"时间设置",24,1);
		Show_Str(428,125,800,16,"例如:12:03:40",16,1);	
		LCD_ShowString(310,174,800,24,24,"  :  :  ");
	}		
	else if(set_numkey==4)
	{
		Show_Str(320,121,800,24,"请输入6位数密码",24,1);		
	}
	else if(set_numkey==5||set_numkey==6)
	{
		Show_Str(320,121,800,24,"标定系数:1-1000",24,1);		
	}
	POINT_COLOR = BLUE;

	Put_AnJian(364, 239, 80, 40);//1
	Show_Str(358,227,24,24,"1",24,1);
	
	Put_AnJian(468, 239, 80, 40);//2
	Show_Str(462,227,24,24,"2",24,1);
	
	Put_AnJian(572, 239, 80, 40);//3
	Show_Str(566,227,24,24,"3",24,1);
	
	Put_AnJian(676, 239, 80, 40);//4
	Show_Str(670,227,24,24,"4",24,1);
	
	Put_AnJian(364, 292, 80, 40);//5
	Show_Str(358,280,24,24,"5",24,1);
	
	Put_AnJian(468, 292, 80, 40);//6
	Show_Str(462,280,24,24,"6",24,1);
	
	Put_AnJian(572, 292, 80, 40);//7
	Show_Str(566,280,24,24,"7",24,1);
	
	Put_AnJian(676, 292, 80, 40);//8
	Show_Str(670,280,24,24,"8",24,1);	
	
	Put_AnJian(364, 345, 80, 40);//9
	Show_Str(358,333,24,24,"9",24,1);

	Put_AnJian(468, 345, 80, 40);//0
	Show_Str(462,333,24,24,"0",24,1);
	
	Put_AnJian(572, 345, 80, 40);		
	Show_Str(548,333,800,24,"清零",24,1);
	
	Put_AnJian(676, 345, 80, 40);	
	Show_Str(652,333,800,24,"确认",24,1);		
	POINT_COLOR = BLACK;
}

//键盘扫描
void ScanKeyBoard(void)
{
	u8 t = 0;
	FT5206_Scan();
	if(ctp_dev.tpsta&TP_PRES_DOWN)//有按键按下并松手
	{
		if((ctp_dev.x[t]>324)&&(ctp_dev.x[t]<404)&&(ctp_dev.y[t]>219)&&(ctp_dev.y[t]<259))//  1
				{						
					key_value=1;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
					
				}

		else if((ctp_dev.x[t]>428)&&(ctp_dev.x[t]<508)&&(ctp_dev.y[t]>219)&&(ctp_dev.y[t]<259))// 2
				{						
					key_value=2;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms		
					
				}

		else if((ctp_dev.x[t]>532)&&(ctp_dev.x[t]<612)&&(ctp_dev.y[t]>219)&&(ctp_dev.y[t]<259))// 3
				{						
					key_value=3;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms		
					
				}
		else if((ctp_dev.x[t]>636)&&(ctp_dev.x[t]<716)&&(ctp_dev.y[t]>219)&&(ctp_dev.y[t]<259))// 4
				{						
					key_value=4;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms		
					
				}
		else if((ctp_dev.x[t]>324)&&(ctp_dev.x[t]<404)&&(ctp_dev.y[t]>272)&&(ctp_dev.y[t]<312))// 5
				{						
					key_value=5;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms		
					
				}
		else if((ctp_dev.x[t]>428)&&(ctp_dev.x[t]<508)&&(ctp_dev.y[t]>272)&&(ctp_dev.y[t]<312))//6
				{						
					key_value=6;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms		
					
				}
		else if((ctp_dev.x[t]>532)&&(ctp_dev.x[t]<612)&&(ctp_dev.y[t]>272)&&(ctp_dev.y[t]<312))// 7
				{						
					key_value=7;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms		
					
				}	
		else if((ctp_dev.x[t]>636)&&(ctp_dev.x[t]<716)&&(ctp_dev.y[t]>272)&&(ctp_dev.y[t]<312))// 8
				{						
					key_value=8;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms		
					
				}
		else if((ctp_dev.x[t]>324)&&(ctp_dev.x[t]<404)&&(ctp_dev.y[t]>325)&&(ctp_dev.y[t]<365))// 9
				{						
					key_value=9;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms		
					
				}

		else if((ctp_dev.x[t]>428)&&(ctp_dev.x[t]<508)&&(ctp_dev.y[t]>325)&&(ctp_dev.y[t]<365))// 0
				{						
					key_value=10;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms		
					
				}
		else if((ctp_dev.x[t]>532)&&(ctp_dev.x[t]<612)&&(ctp_dev.y[t]>325)&&(ctp_dev.y[t]<365))// 清零
				{						
					key_value=11;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms		
					
				}		
		else if((ctp_dev.x[t]>636)&&(ctp_dev.x[t]<716)&&(ctp_dev.y[t]>325)&&(ctp_dev.y[t]<365))// 确认
				{						
					key_value=12;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms		
					
				}	
		else if((ctp_dev.x[t]>690)&&(ctp_dev.x[t]<730)&&(ctp_dev.y[t]>113)&&(ctp_dev.y[t]<153))// 退出
				{						
					key_value=13;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms		
					
				}				
		else
				{
					key_value=0;			
					
				}
	}

}



//键盘设置界面
//set_numkey  1：试样编号设置
//						2：日期设置
//						3：时间设置
//						4：校准密码设置
//						5：标定点1
//						6：标定点2
void menu_setkeyboard(void)
{
	u8 i=1,j=0;
	u16 num=0;
	u16 setyears;//设定的年
	u8 setmonth;//设定月
	u8 setday;	//设定年
	u8 sethour;//设定的时
	u8 setmin;//设定分
	u8 setsec;	//设定秒
	u32 password_num=0;//密码
	u16 biaoding_num=0;
	static u8 mdata[8]={0};	
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTGRAY;
	JianPan_Displayfu();//显示虚拟键盘
	while(1)
	{	
		if(i==0) return;//退出大循环	
		ScanKeyBoard();
		if((key_value>0)&&(key_value<11))
		{
			if(key_value==10)
				mdata[j]=key_value-10;
			else 
				mdata[j]=key_value;			
			if(mdata[0]==0&&(set_numkey<3||set_numkey>4))//第一位为0
			{
				Show_Str(520,174,800,24,"Setting Error!",24,1);						
				continue;
			}
			j++;
			if(set_numkey==1&&j>5) //编号设置
			{
				Show_Str(520,174,800,24,"out of rang!",24,1);				
				delay_ms(1000);;	//延时1S	
				Show_Str(520,174,800,24,"            ",24,1);			
				key_value=0;
				j=5;		
				continue;
			}
			else if(set_numkey==2&&j>8) //日期设置
			{
				Show_Str(520,174,800,24,"out of rang!",24,1);				
				delay_ms(1000);;	//延时1S	
				Show_Str(520,174,800,24,"            ",24,1);				
				key_value=0;
				j=8;		
				continue;
			}
			else if((set_numkey==3||set_numkey==4)&&j>6) //时间设置和AD密码设置
			{
				Show_Str(520,174,800,24,"out of rang!",24,1);				
				delay_ms(1000);;	//延时1S	
				Show_Str(520,174,800,24,"            ",24,1);					
				key_value=0;
				j=6;		
				continue;
			}
			else if((set_numkey==5||set_numkey==6)&&j>4) //标定系数
			{
				Show_Str(520,174,800,24,"out of rang!",24,1);				
				delay_ms(1000);;	//延时1S	
				Show_Str(520,174,800,24,"            ",24,1);				
				key_value=0;
				j=4;		
				continue;
			}
		}
		switch(key_value)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:					
			case 9:
			case 10://0
					key_value=0;
					if(set_numkey==1)//编号设置
						LCD_ShowxNum(310+(j-1)*12,174,mdata[j-1],1,24,0x80);
					else if(set_numkey==2)//日期设置
						LCD_ShowNum(310+(j-1)*12,174,mdata[j-1],1,24);	
					else if(set_numkey==3)	//时间设置
					{
						if((j>2)&&(j<5))
							LCD_ShowxNum(324+(j-1)*12,174,mdata[j-1],1,24,0x80);
						else if(j>4)
							LCD_ShowxNum(336+(j-1)*12,174,mdata[j-1],1,24,0x80);
						else
							LCD_ShowxNum(310+(j-1)*12,174,mdata[j-1],1,24,0x80);
					}
					else if(set_numkey==4||set_numkey==5||set_numkey==6)//密码 标定系数
						LCD_ShowNum(310+(j-1)*12,174,mdata[j-1],1,24);					
					break;
			case 11://清零					
					for(j=0;j<8;j++) mdata[j]=0;
					j=0;					        
					LCD_ShowString(520,174,800,24,24,"              ");	
					LCD_ShowString(310,174,800,24,24,"        ");					
					key_value=0;
					if(set_numkey==3) 
						LCD_ShowString(310,174,800,24,24,"  :  :  ");
					break;
			
			case 12://确认
					if(set_numkey==1)//编号
					{	
						if(j==1) 				num=mdata[0];
						else if(j==2) 	num=mdata[0]*10+mdata[1];
						else if(j==3) 	num=mdata[0]*100+mdata[1]*10+mdata[2];
						else if(j==4) 	num=mdata[0]*1000+mdata[1]*100+mdata[2]*10+mdata[3];
						else if(j==5) 	num=mdata[0]*10000+mdata[1]*1000+mdata[2]*100+mdata[3]*10+mdata[4];		
						if(num>9999)
						{
							Show_Str(520,174,800,24,"Setting Error!",24,1);						
							continue;	
						}
						else
						{
							//保存到EEPROM
							number=num;														
							i=0;//设置正确直接返回
						}
					}
					else if(set_numkey==2)//日期设置
					{	
						setyears=mdata[0]*1000+mdata[1]*100+mdata[2]*10+mdata[3];
						setmonth=mdata[4]*10+mdata[5];						
						setday=mdata[6]*10+mdata[7];
						if(setyears>2099||setmonth>12||setmonth==0||setday>31||setday==0)
						{
							Show_Str(520,174,800,24,"Setting Error!",24,1);						
							continue;	
						}
						else
						{
							RTC_Set(setyears,setmonth,setday,calendar.hour,calendar.min,calendar.sec);													
							i=0;//设置正确直接返回
						}
					}
					else if(set_numkey==3)
					{
						sethour=mdata[0]*10+mdata[1];						
						setmin=mdata[2]*10+mdata[3];						
						setsec=mdata[4]*10+mdata[5];					
						
						if(sethour>24||setmin>60||setsec>60) 
						{
							Show_Str(520,174,800,24,"Setting Error!",24,1);									
							continue;
						}
						else//设置正确
						{
							RTC_Set(calendar.w_year,calendar.w_month,calendar.w_date,sethour,setmin,setsec);
							i=0;//设置正确直接返回
						}
					}
					else if(set_numkey==4)//AD密码
					{	
						if(j==1) 				password_num=mdata[0];
						else if(j==2) 	password_num=mdata[0]*10+mdata[1];
						else if(j==3) 	password_num=mdata[0]*100+mdata[1]*10+mdata[2];
						else if(j==4) 	password_num=mdata[0]*1000+mdata[1]*100+mdata[2]*10+mdata[3];
						else if(j==5) 	password_num=mdata[0]*10000+mdata[1]*1000+mdata[2]*100+mdata[3]*10+mdata[4];
						else if(j==6)	password_num=mdata[0]*100000+mdata[1]*10000+mdata[2]*1000+mdata[3]*100+mdata[4]*10+mdata[5];						
						if(password_num!=password)
						{
							Show_Str(520,174,800,24,"Setting Error!",24,1);						
							continue;	
						}
						else
						{							
							passwordcorrect=1;							
							i=0;//设置正确直接返回
						}
					}
					else if(set_numkey==5||set_numkey==6)//标定系数
					{	
						if(j==1) 				biaoding_num=mdata[0];
						else if(j==2) 	biaoding_num=mdata[0]*10+mdata[1];
						else if(j==3) 	biaoding_num=mdata[0]*100+mdata[1]*10+mdata[2];
						else if(j==4) 	biaoding_num=mdata[0]*1000+mdata[1]*100+mdata[2]*10+mdata[3];
												
						if(biaoding_num>1000)
						{
							Show_Str(520,174,800,24,"Setting Error!",24,1);						
							continue;	
						}
						else
						{							
							if(set_numkey==5) 				biaoding1=biaoding_num;
							else if(set_numkey==6) 		biaoding2=biaoding_num;	
							i=0;//设置正确直接返回
						}
					}	
					key_value=0;
					break;
			case 13://返回
					i=0;//退出大循环					
					key_value=0;
					break;
			default: break;			
		}
	}	
}

//帮助界面
void menu_help(void)
{
	u8 t=0;
	u8 flag=1;
	LCD_Fill(240,80,800,480,CYAN);//清掉之前的汉字
	Put_AnJian(520,240,440,280);//键盘大边框;
	Show_Str(300,120,800,24,"公司地址:东湖高新区高新二路25号",24,1);
	Show_Str(300,164,800,24,"邮政编码:430079",24,1);
	Show_Str(300,208,800,24,"联 系 人:项经理",24,1);
	Show_Str(300,252,800,24,"联系电话:027-87467572",24,1);
	Show_Str(300,296,800,24,"移动电话:18802726499",24,1);
	Show_Str(300,340,800,24,"公司传真:027-51767377",24,1);
	while(flag)
	{
			t=0;
			if(RTC_500MS)
			{
				RTC_500MS=0;			
				display_time();//显示时间
			}	
			FT5206_Scan();		
			if(ctp_dev.tpsta&TP_PRES_DOWN)		//触摸屏被按下
			{	
				if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>78)&&(ctp_dev.y[t]<128))//设置
					{				
						key_value=1;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						AnJianPressed(114,103,114,50);
						Show_Str(77,87,800,32,"设置",32,1);//设置按键按下
						Put_AnJian(114,423,114,50);
						Show_Str(77,407,800,32,"帮助",32,1);//帮助键弹起						
					}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>158)&&(ctp_dev.y[t]<208))//试验
					{				
						key_value=2;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						Put_AnJian(114,423,114,50);
						Show_Str(77,407,800,32,"帮助",32,1);//帮助键弹起
						AnJianPressed(114,183,114,50);
						Show_Str(77,167,800,32,"试验",32,1);						
					}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>238)&&(ctp_dev.y[t]<288))//数据
					{				
						key_value=3;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						Put_AnJian(114,423,114,50);
						Show_Str(77,407,800,32,"帮助",32,1);//帮助键弹起
						AnJianPressed(114,263,114,50);
						Show_Str(77,247,800,32,"数据",32,1);						
					}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>318)&&(ctp_dev.y[t]<368))//校准
					{						
						key_value=4;//	停止按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms
						Put_AnJian(114,423,114,50);
						Show_Str(77,407,800,32,"帮助",32,1);//帮助键弹起
						AnJianPressed(114,343,114,50);
						Show_Str(77,327,800,32,"校准",32,1);
					}				
				else	key_value=0;					
		  }
			else//松开屏幕 按下确认或者返回键都退出循环
			{			
				if(key_value!=0)
				{			
					switch(key_value)
					{						
						case 1://设置界面
							key_value=0;
							menu_setting();
							menu_setting_key();
							flag=0;
							break;	
						case 2://试验
							key_value=0;
							menu1();
							flag=0;
							break;
						case 3://数据
							key_value=0;
							menu_datacheck();
							menu_datacheck_key();
							flag=0;
							break;
						case 4://校准
							key_value=0;
							menu_password();
							flag=0;							
							break;
						default:
							break;
					}
					key_value=0;//清除按键标志
				}
			}		
	}
}
//进入AD密码界面
void menu_password(void)
{
	u8 t=0;
	u8 flag=1;
	LCD_Fill(240,80,800,480,CYAN);//清掉之前的汉字
	Put_AnJian(586,240,100,30);//键盘大边框;
	Show_Str(392,228,800,24,"请输入密码:",24,1);
	while(flag)
	{
			t=0;
			if(RTC_500MS)
			{
				RTC_500MS=0;			
				display_time();//显示时间
			}	
			FT5206_Scan();		
			if(ctp_dev.tpsta&TP_PRES_DOWN)		//触摸屏被按下
			{	
				if((ctp_dev.x[t]>536)&&(ctp_dev.x[t]<636)&&(ctp_dev.y[t]>225)&&(ctp_dev.y[t]<255))//输入密码
					{				
						key_value=1;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
											
					}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>78)&&(ctp_dev.y[t]<128))//设置
					{				
						key_value=2;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						AnJianPressed(114,103,114,50);
						Show_Str(77,87,800,32,"设置",32,1);//设置按键按下						
						Put_AnJian(114,343,114,50);
						Show_Str(77,327,800,32,"校准",32,1);//校准按键弹起						
					}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>158)&&(ctp_dev.y[t]<208))//试验
					{				
						key_value=3;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						Put_AnJian(114,343,114,50);
						Show_Str(77,327,800,32,"校准",32,1);//校准按键弹起
						AnJianPressed(114,183,114,50);
						Show_Str(77,167,800,32,"试验",32,1);						
					}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>238)&&(ctp_dev.y[t]<288))//数据
					{				
						key_value=4;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						Put_AnJian(114,343,114,50);
						Show_Str(77,327,800,32,"校准",32,1);//校准按键弹起
						AnJianPressed(114,263,114,50);
						Show_Str(77,247,800,32,"数据",32,1);						
					}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>398)&&(ctp_dev.y[t]<448))//帮助
					{						
						key_value=5;//	停止按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms
						Put_AnJian(114,343,114,50);
						Show_Str(77,327,800,32,"校准",32,1);//校准按键弹起
						AnJianPressed(114,423,114,50);
						Show_Str(77,407,800,32,"帮助",32,1);
					}
			
				else	key_value=0;					
		  }
			else//松开屏幕 按下确认或者返回键都退出循环
			{			
				if(key_value!=0)
				{			
					switch(key_value)
					{						
						case 1:
							key_value=0;
							set_numkey=4;
							menu_setkeyboard();
							set_numkey=0;
							if(passwordcorrect)//判断密码是否正确
							{
								passwordcorrect=0;
								AD_Calibration_menu();
								flag=0;
							}
							else
							{
								LCD_Fill(240,80,800,480,CYAN);//清掉之前的汉字
								Put_AnJian(586,240,100,30);//键盘大边框;
								Show_Str(392,228,800,24,"请输入密码:",24,1);
							}							
							break;
						case 2://进入设置界面
							key_value=0;
							menu_setting();
							menu_setting_key();
							flag=0;
							break;
						case 3://试验界面
							key_value=0;
							menu1();
							flag=0;
							break;
						case 4://数据
							key_value=0;
							menu_datacheck();
							menu_datacheck_key();
							flag=0;
							break;
						case 5://帮助
							key_value=0;
							menu_help();
							flag=0;							
							break;	
						default:
							break;
					}
					key_value=0;//清除按键标志
				}
			}		
	}
}
//进入AD校准界面
void Calibration_menu(void)
{
	char a[8]={0};
	
	u16 backup_color;
	LCD_Fill(240,80,800,480,CYAN);//清掉之前的汉字
	Show_Str(293,136,800,24,"标定点1:",24,1);
	Show_Str(546,136,800,24,"标定点2:",24,1);
	LCD_Fill(393,130,493,166,WHITE);
	LCD_Fill(646,130,746,166,WHITE);
	
	Show_Str(293,196,800,24,"AD1:",24,1);
	Show_Str(546,196,800,24,"AD2:",24,1);
	LCD_Fill(393,190,493,226,WHITE);
	LCD_Fill(646,190,746,226,WHITE);

	Show_Str(460,256,800,24,"K:",24,1);
	Show_Str(460,316,800,24,"C:",24,1);

	backup_color=BACK_COLOR;
	BACK_COLOR=WHITE;
	sprintf(a,"%-04d",biaoding1);
	LCD_ShowString(413,136,800,24,24,(u8 *)a);
	sprintf(a,"%-04d",biaoding2);
	LCD_ShowString(666,136,800,24,24,(u8 *)a);

	sprintf(a,"%-05d",AD1value);
	LCD_ShowString(413,196,800,24,24,(u8 *)a);
	sprintf(a,"%-05d",AD2value);
	LCD_ShowString(666,196,800,24,24,(u8 *)a);
	BACK_COLOR=CYAN;
	sprintf(a,"%-8.4f",press_k);
	LCD_ShowString(484,256,800,24,24,(u8 *)a);
	sprintf(a,"%-8.4f",press_c);
	LCD_ShowString(484,316,800,24,24,(u8 *)a);
	BACK_COLOR=LIGHTGRAY;
	
	Put_AnJian(520,423,114,50);
	Show_Str(488,407,800,32,"校准",32,1);
	
	BACK_COLOR=backup_color;
}

//AD校准界面
void AD_Calibration_menu(void)
{
	u8 flag1=0,flag2=0;
	u8 t = 0;
	u8 flag=1;
	char value[8]={0};	
	u8 b[16]={0};
		//读取保存的压力AD系数
	AT24CXX_Read(b,17,16);
	pressAD_kint=(b[0]<<8)+b[1];
	pressAD_kfloat=(b[2]<<8)+b[3];
	pressAD_cint=(b[4]<<8)+b[5];
	pressAD_cfloat=(b[6]<<8)+b[7];

	press_k=(float)pressAD_kint+((float)pressAD_kfloat/10000);
	press_c=(float)pressAD_cint+((float)pressAD_cfloat/10000);
	biaoding1=(b[8]<<8)+b[9];	
	AD1value=(b[10]<<8)+b[11];
	biaoding2=(b[12]<<8)+b[13];	
	AD2value=(b[14]<<8)+b[15];
	Calibration_menu();		
	while(1)
	{
		t=0;
		if(RTC_500MS)
		{
			RTC_500MS=0;			
			display_time();//显示时间
		}	
		FT5206_Scan();	
		if(flag==0) return;//退出大循环	
		if(ctp_dev.tpsta&TP_PRES_DOWN)//触摸屏被按下
		{	
			
			if((ctp_dev.x[t]>463)&&(ctp_dev.x[t]<577)&&(ctp_dev.y[t]>398)&&(ctp_dev.y[t]<448))//校准按钮被按下
				{						
					key_value=1;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
					
				}		
			else if((ctp_dev.x[t]>393)&&(ctp_dev.x[t]<493)&&(ctp_dev.y[t]>130)&&(ctp_dev.y[t]<166)&&flag2==0)//标定点1设置
				{						
					key_value=2;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms						
					
				}
			else if((ctp_dev.x[t]>393)&&(ctp_dev.x[t]<493)&&(ctp_dev.y[t]>190)&&(ctp_dev.y[t]<226)&&flag2==0)//AD1采样
				{						
					key_value=3;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
					
				}
			else if((ctp_dev.x[t]>646)&&(ctp_dev.x[t]<746)&&(ctp_dev.y[t]>130)&&(ctp_dev.y[t]<166)&&flag1==0)//标定点2
				{						
					key_value=4;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
					
				}	
			else if((ctp_dev.x[t]>646)&&(ctp_dev.x[t]<746)&&(ctp_dev.y[t]>190)&&(ctp_dev.y[t]<226)&&flag1==0)//AD2采样
				{						
					key_value=5;
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms						
				}
			else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>78)&&(ctp_dev.y[t]<128))//设置
					{				
						key_value=6;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						AnJianPressed(114,103,114,50);
						Show_Str(77,87,800,32,"设置",32,1);//设置按键按下						
						Put_AnJian(114,343,114,50);
						Show_Str(77,327,800,32,"校准",32,1);//校准按键弹起						
					}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>158)&&(ctp_dev.y[t]<208))//试验
					{				
						key_value=7;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						Put_AnJian(114,343,114,50);
						Show_Str(77,327,800,32,"校准",32,1);//校准按键弹起
						AnJianPressed(114,183,114,50);
						Show_Str(77,167,800,32,"试验",32,1);						
					}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>238)&&(ctp_dev.y[t]<288))//数据
					{				
						key_value=8;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						Put_AnJian(114,343,114,50);
						Show_Str(77,327,800,32,"校准",32,1);//校准按键弹起
						AnJianPressed(114,263,114,50);
						Show_Str(77,247,800,32,"数据",32,1);						
					}
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>398)&&(ctp_dev.y[t]<448))//帮助
					{						
						key_value=9;//	停止按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms
						Put_AnJian(114,343,114,50);
						Show_Str(77,327,800,32,"校准",32,1);//校准按键弹起
						AnJianPressed(114,423,114,50);
						Show_Str(77,407,800,32,"帮助",32,1);
					}		
			else
				{
					key_value=0;					
				}	
		}
		else
		{
			if(key_value!=0)
				{					
					switch(key_value)
					{
						case 1:  //校准计算 并显示计算出的K值
							press_k=(float)(biaoding1-biaoding2)/(AD1value-AD2value);
							press_c=((float)(biaoding1+biaoding2)-press_k*(AD1value+AD2value))/2;
							pressAD_kint=press_k;
							pressAD_kfloat=(press_k-pressAD_kint)*10000;
							pressAD_cint=press_c;
							pressAD_cfloat=(press_c-pressAD_cint)*10000;
							sprintf(value,"%-8.4f",press_k);
							LCD_ShowString(484,256,800,24,24,(u8 *)value);
							sprintf(value,"%-8.4f",press_c);
							LCD_ShowString(484,316,800,24,24,(u8 *)value);
							//保存系数K
							b[0]=pressAD_kint>>8;
							b[1]=pressAD_kint;
							b[2]=pressAD_kfloat>>8;
							b[3]=pressAD_kfloat;
							
							//保存系数C
							b[4]=pressAD_cint>>8;
							b[5]=pressAD_cint;
							b[6]=pressAD_cfloat>>8;
							b[7]=pressAD_cfloat;
							
							//biaoding1
							b[8]=biaoding1>>8;
							b[9]=biaoding1;
							//ad1
							b[10]=AD1value>>8;
							b[11]=AD1value;
							//biaoding2
							b[12]=biaoding2>>8;
							b[13]=biaoding2;
							//ad2
							b[14]=AD2value>>8;
							b[15]=AD2value;
							AT24CXX_Write(b,17,16)	;						
							key_value = 0;
							flag1=0;
							flag2=0;		
							break;											
						case 2:  //标定点1
							key_value = 0;	
							set_numkey=5;							
							menu_setkeyboard();
							Calibration_menu();
							set_numkey=0;
							flag1=1;
							break;		
						case 3:  //AD1采样
							key_value = 0;
							AD1value=Get_Adc_Average(10, 50);
							Calibration_menu();
							flag1=0;
							break;
						case 4:  //标定点2								
							key_value = 0;	
							set_numkey=6;
							menu_setkeyboard();
							Calibration_menu();
							set_numkey=0;
							flag2=1;
							break;
						case 5:  //AD2采样
							key_value = 0;
							AD2value=Get_Adc_Average(10, 50);
							Calibration_menu();
							flag2=0;
							break;
						case 6:  //设置
							key_value=0;
							menu_setting();
							menu_setting_key();
							flag=0;						
							break;
						case 7:  //试验
							key_value=0;
							menu1();
							flag=0;						
							break;
						case 8: //数据 
							key_value=0;
							menu_datacheck();
							menu_datacheck_key();
							flag=0;						
							break;
						case 9://帮助  
							key_value = 0;								
							menu_help();						
							break;
						default:
							break;
					}				
					
				}
		}

	}

}
void Show_Excel(void)
{	
	LCD_DrawRectangle(300,80,740,360);//画表格外框
	//画水平线
	gui_draw_hline(300,120,440,BLACK);
	gui_draw_hline(300,160,440,BLACK);
	gui_draw_hline(300,200,440,BLACK);
	gui_draw_hline(300,240,440,BLACK);
	gui_draw_hline(300,280,440,BLACK);
	gui_draw_hline(300,320,440,BLACK);
	//画一条垂直线
	LCD_Fill(500,81,540,359,CYAN);//清掉之前的汉字
	gui_draw_vline(540,80,280,BLACK);
	gui_draw_vline(500,80,280,BLACK);
	gui_draw_vline(400,80,280,BLACK);
	gui_draw_vline(640,120,240,BLACK);	
	//显示文字
	Show_Str(300,88,800,24,"试样编号",24,1);
	Show_Str(300,128,800,24,"操作员号",24,1);
	Show_Str(300,168,800,24,"试样日期",24,1);
	Show_Str(300,208,800,24,"试样时间",24,1);
	Show_Str(300,248,800,24,"测试流量",24,1);
	Show_Str(300,288,800,24,"吸气阻力",24,1);
	Show_Str(300,328,800,24," 气溶胶 ",24,1);
	LCD_Fill(639,81,641,119,CYAN);//清掉之前的汉字
	Show_Str(574,88,800,24,"过滤效率(%)",24,1);
	Show_Str(560,128,800,24,"0.3um",24,1);
	Show_Str(560,168,800,24,"0.5um",24,1);
	Show_Str(560,208,800,24,"1.0um",24,1);
	Show_Str(560,248,800,24,"3.0um",24,1);
	Show_Str(560,288,800,24,"5.0um",24,1);
	Show_Str(566,328,800,24,"10um",24,1);
}
//数据菜单
void menu_datacheck(void)
{	
	u16 backup_color;
	u16 Data_Dem=0;
	u16	People_Test=0;
	u16	years=0;
	float flow_data=0;
	char	a[5]={0};
	backup_color=BACK_COLOR;
	BACK_COLOR=CYAN;
	LCD_Fill(240,80,800,480,CYAN);//清掉之前的汉字
	//插入表格	
	Show_Excel();
	//插入四个按钮
	Put_AnJian(328,423,80,40);//上查
	Put_AnJian(456,423,80,40);//下查
	Put_AnJian(584,423,80,40);//清空
	Put_AnJian(712,423,80,40);//打印	
	//显示汉字
	Show_Str(304,411,800,24,"上查",24,1);
	Show_Str(432,411,800,24,"下查",24,1);
	Show_Str(560,411,800,24,"清空",24,1);
	Show_Str(688,411,800,24,"打印",24,1);
	//显示数据	
	SPI_Flash_Read(&Dust.Data_years[0],(Data_checkNum-1)*256,26);	
	//试样编号
	if(Dust.num==0xff)	Dust.num=0;
	LCD_ShowNum(438,88,Dust.num,2,24);
	//操作员号	
	People_Test=((((u16)Dust.People[0])<<8)+Dust.People[1]);
	if(People_Test==0xffff)	People_Test=0;
	LCD_ShowxNum(426,128,People_Test,4,24,0X80);
	//年月日	
	years=((((u16)Dust.Data_years[0])<<8)+Dust.Data_years[1]);
	if(years==0xffff)	years=0;
	if(Dust.Data_month==0xff)	Dust.Data_month=0;
	if(Dust.Data_day==0xff)	Dust.Data_day=0;
	LCD_ShowxNum(402,168,years,4,24,0X80);
	LCD_ShowxNum(450,168,Dust.Data_month,2,24,0X80);
	LCD_ShowxNum(474,168,Dust.Data_day,2,24,0X80);
	//时分秒
	if(Dust.Hours==0xff)			Dust.Hours=0;
	if(Dust.Minutes==0xff)		Dust.Minutes=0;
	if(Dust.Seconds==0xff)		Dust.Seconds=0;
	LCD_ShowString(402,208,800,24,24,"  :  :  ");//时间
	LCD_ShowxNum(402,208,Dust.Hours,2,24,0X80);
	LCD_ShowxNum(438,208,Dust.Minutes,2,24,0X80);
	LCD_ShowxNum(474,208,Dust.Seconds,2,24,0X80);
	//流量
	if(Dust.Flow==0xff)	flow_data=0;
	else
		{	
			if(Dust.Flow==0)								flow_data=32.0f;
			else if(Dust.Flow==1)						flow_data=85.0f;	
			else if(Dust.Flow==2)						flow_data=95.0f;	
		}	
	sprintf(a,"%04.1f",flow_data);
	LCD_ShowString(426,248,800,24,24,(u8 *)a);
	//吸气阻力
	Data_Dem=((((u16)Dust.Press[0])<<8)+Dust.Press[1]);
	if(Data_Dem==0xffff)	Data_Dem=0;
	LCD_ShowxNum(426,288,Data_Dem,4,24,0X80);
	//气溶胶类型
	if(Dust.Aerosol==0xff)	Show_Str(426,328,800,24,"0000",24,1);
	else
	{
		if(Dust.Aerosol==0)	Show_Str(426,328,800,24,"NACL",24,1);
		else								Show_Str(426,328,800,24,"DEHS",24,1);
	}
	//过滤效率
	//0.3
	Data_Dem=((((u16)Dust.Efficiency_03[0])<<8)+Dust.Efficiency_03[1]);
	if(Data_Dem==0xffff)	Data_Dem=0;
	flow_data=((float)Data_Dem)/100;
	sprintf(a,"%-5.2f",flow_data);
	LCD_ShowString(660,128,800,24,24,(u8 *)a);
	//0.5
	Data_Dem=((((u16)Dust.Efficiency_05[0])<<8)+Dust.Efficiency_05[1]);
	if(Data_Dem==0xffff)	Data_Dem=0;
	flow_data=((float)Data_Dem)/100;
	sprintf(a,"%-5.2f",flow_data);
	LCD_ShowString(660,168,800,24,24,(u8 *)a);
	//1
	Data_Dem=((((u16)Dust.Efficiency_1[0])<<8)+Dust.Efficiency_1[1]);
	if(Data_Dem==0xffff)	Data_Dem=0;
	flow_data=((float)Data_Dem)/100;
	sprintf(a,"%-5.2f",flow_data);
	LCD_ShowString(660,208,800,24,24,(u8 *)a);
	//3
	Data_Dem=((((u16)Dust.Efficiency_3[0])<<8)+Dust.Efficiency_3[1]);
	if(Data_Dem==0xffff)	Data_Dem=0;
	flow_data=((float)Data_Dem)/100;
	sprintf(a,"%-5.2f",flow_data);
	LCD_ShowString(660,248,800,24,24,(u8 *)a);
	//5
	Data_Dem=((((u16)Dust.Efficiency_5[0])<<8)+Dust.Efficiency_5[1]);
	if(Data_Dem==0xffff)	Data_Dem=0;
	flow_data=((float)Data_Dem)/100;
	sprintf(a,"%-5.2f",flow_data);
	LCD_ShowString(660,288,800,24,24,(u8 *)a);
	//10
	Data_Dem=((((u16)Dust.Efficiency_10[0])<<8)+Dust.Efficiency_10[1]);
	if(Data_Dem==0xffff)	Data_Dem=0;
	flow_data=((float)Data_Dem)/100;
	sprintf(a,"%-5.2f",flow_data);
	LCD_ShowString(660,328,800,24,24,(u8 *)a);
	BACK_COLOR=backup_color;
}
//数据界面的按键设置
void menu_datacheck_key(void)
{
	u8 t;
	u8 flag=1;	
	while(flag)
	{
			t=0;
			if(RTC_500MS)
			{
				RTC_500MS=0;			
				display_time();//显示时间
			}	
			FT5206_Scan();		
			if(ctp_dev.tpsta&TP_PRES_DOWN)		//触摸屏被按下
			{	   	
				if((ctp_dev.x[t]>288&&ctp_dev.x[t]<368)&&(ctp_dev.y[t]>403&&ctp_dev.y[t]<443))	//上查
				{
					key_value=1;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else if((ctp_dev.x[t]>416&&ctp_dev.x[t]<496)&&(ctp_dev.y[t]>403&&ctp_dev.y[t]<443))	//下查
				{
					key_value=2;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else if((ctp_dev.x[t]>544&&ctp_dev.x[t]<624)&&(ctp_dev.y[t]>403&&ctp_dev.y[t]<443))	//清空
				{
					key_value=3;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}
				else if((ctp_dev.x[t]>672&&ctp_dev.x[t]<752)&&(ctp_dev.y[t]>403&&ctp_dev.y[t]<443))	//打印
				{
					key_value=4;//设置按键标志
					Loosen_Lcd();  //等待按键松开
					speekers(100);//蜂鸣器响100ms	
				}	
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>78)&&(ctp_dev.y[t]<128))//设置
					{				
						key_value=5;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						AnJianPressed(114,103,114,50);
						Show_Str(77,87,800,32,"设置",32,1);//设置按键按下					
						Put_AnJian(114,263,114,50);
						Show_Str(77,247,800,32,"数据",32,1);//数据键弹起	
					}	
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>158)&&(ctp_dev.y[t]<208))//试验
					{				
						key_value=6;//	菜单按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms					
						Put_AnJian(114,263,114,50);
						Show_Str(77,247,800,32,"数据",32,1);//数据键弹起
						AnJianPressed(114,183,114,50);
						Show_Str(77,167,800,32,"试验",32,1);
					}				
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>318)&&(ctp_dev.y[t]<368))//校准
					{						
						key_value=7;//	停止按键标志
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms
						Put_AnJian(114,263,114,50);
						Show_Str(77,247,800,32,"数据",32,1);//数据键弹起
						AnJianPressed(114,343,114,50);
						Show_Str(77,327,800,32,"校准",32,1);
					}	
				else if((ctp_dev.x[t]>57)&&(ctp_dev.x[t]<171)&&(ctp_dev.y[t]>398)&&(ctp_dev.y[t]<448))//帮助
					{					
						key_value=8;//	测试按键标志					
						Loosen_Lcd();  //等待按键松开
						speekers(100);//蜂鸣器响100ms	
						Put_AnJian(114,263,114,50);
						Show_Str(77,247,800,32,"数据",32,1);//数据键弹起
						AnJianPressed(114,423,114,50);
						Show_Str(77,407,800,32,"帮助",32,1);
						
					}
				else	key_value=0;					
		  }
			else//松开屏幕 按下确认或者返回键都退出循环
			{			
				if(key_value!=0)
				{			
					switch(key_value)
					{
						case 1:  //上查			
							key_value = 0;
							if(Data_checkNum>64||Data_checkNum>=Test_num-1) 	Data_checkNum=1;
							else									Data_checkNum++;
							menu_datacheck();
							break;
						case 2://下查
							key_value=0;
							if(Data_checkNum==1) 	Data_checkNum=Test_num-1;
							else									Data_checkNum--;
							menu_datacheck();
							break;	
						case 3://清空 
							key_value=0;
							Erase_Waring();
							menu_datacheck();
							break;
						case 4://打印
							key_value=0;
							PrintReport();										
							break;
						case 5://设置
							key_value=0;
							menu_setting();
							menu_setting_key();
							flag=0;	
							Data_checkNum=1;
							break;
						case 6://试验
							key_value=0;
							menu1();
							flag=0;
							Data_checkNum=1;
							break;						
						case 7://校准
							key_value=0;
							menu_password();
							flag=0;
							Data_checkNum=1;
							break;
						case 8://帮助
							key_value=0;							
							menu_help();
							flag=0;//退出本循环
							Data_checkNum=1;
							break;
						default:
							break;
					}
					key_value=0;//清除按键标志
				}
			}		
	}
}


//打印报表
void PrintReport(void)
{		
	u16	a=0;
	float	Eff=0;	
	char Data[5]={0};
//打印机初始化
    pprint(0x1b);                                                                                      
    pprint(0x40);
// 		pprint(0x1b);                                                                                      
//     pprint(0x51);
// 		pprint(0x00);	
		if(Dust.num!=0xff)
		{	
		//---------------------
		PrintStr("---------------");
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
			
		//10um
		PrintStr("@≥10um   ");
		a=((((u16)Dust.Efficiency_10[0])<<8)+Dust.Efficiency_10[1]);
		Eff=((float)a)/100;
		sprintf(Data,"%-5.2f",Eff);		
		PrintStrNnum(Data,5);
		PrintStr("%");
		pprint(0x0d);  //回车
		pprint(0x0a); //换行	
			
		//5um
		PrintStr("@≥5um    ");
		a=((((u16)Dust.Efficiency_5[0])<<8)+Dust.Efficiency_5[1]);
		Eff=((float)a)/100;
		sprintf(Data,"%-5.2f",Eff);		
		PrintStrNnum(Data,5);
		PrintStr("%");
		pprint(0x0d);  //回车
		pprint(0x0a); //换行		
			
		//3um
		PrintStr("@≥3um    ");
		a=((((u16)Dust.Efficiency_3[0])<<8)+Dust.Efficiency_3[1]);
		Eff=((float)a)/100;
		sprintf(Data,"%-5.2f",Eff);		
		PrintStrNnum(Data,5);
		PrintStr("%");
		pprint(0x0d);  //回车
		pprint(0x0a); //换行	
		//1um
		PrintStr("@≥1um    ");
		a=((((u16)Dust.Efficiency_1[0])<<8)+Dust.Efficiency_1[1]);
		Eff=((float)a)/100;
		sprintf(Data,"%-5.2f",Eff);		
		PrintStrNnum(Data,5);
		PrintStr("%");
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
		//0.5um
		PrintStr("@≥0.5um  ");
		a=((((u16)Dust.Efficiency_05[0])<<8)+Dust.Efficiency_05[1]);
		Eff=((float)a)/100;
		sprintf(Data,"%-5.2f",Eff);		
		PrintStrNnum(Data,5);
		PrintStr("%");
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
		//0.3um
		PrintStr("@≥0.3um  ");
		a=((((u16)Dust.Efficiency_03[0])<<8)+Dust.Efficiency_03[1]);
		Eff=((float)a)/100;
		sprintf(Data,"%-5.2f",Eff);		
		PrintStrNnum(Data,5);
		PrintStr("%");
		pprint(0x0d);  //回车
		pprint(0x0a); //换行		
		//---------------------
		PrintStr("----------------");		
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
		PrintStr("       PFE      ");
		PrintStr("----------------");
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
		
		//吸气阻力
		PrintStr("Press:");
		a=((((u16)Dust.Press[0])<<8)+Dust.Press[1]);
		sprintf(Data,"%04d",a);
		PrintStrNnum(Data,4);
		PrintStr("Pa");
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
		//测试流量
		PrintStr("Flow:");	
		if(Dust.Flow==0)				PrintStr("32.0L/Min");	
		else if(Dust.Flow==1)		PrintStr("85.0L/Min");	
		else if(Dust.Flow==2)		PrintStr("95.0L/Min");	
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
		
		//时间
		PrintStr("Time:");	
		sprintf(Data,"%02d",Dust.Hours);
		PrintStrNnum(Data,2);
		PrintStr(":");
		
		sprintf(Data,"%02d",Dust.Minutes);
		PrintStrNnum(Data,2);
		PrintStr(":");
		
		sprintf(Data,"%02d",Dust.Seconds);
		PrintStrNnum(Data,2);
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
		//日期		
		PrintStr("Data:");
		a=((((u16)Dust.Data_years[0])<<8)+Dust.Data_years[1]);
		sprintf(Data,"%04d",a);
		PrintStrNnum(Data,4);
		PrintStr("/");
		
		sprintf(Data,"%02d",Dust.Data_month);
		PrintStrNnum(Data,2);
		PrintStr("/");
		
		sprintf(Data,"%02d",Dust.Data_day);
		PrintStrNnum(Data,2);
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
		//类型	
		PrintStr("Type:");	
		if(Dust.Aerosol==0)	PrintStr("NACL");
		else								PrintStr("DEHS");
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
		//操作员号		
		PrintStr("Operator:");	
		a=((((u16)Dust.People[0])<<8)+Dust.People[1]);
		sprintf(Data,"%04d",a);			
		PrintStrNnum(Data,4);	
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
		//试样编号
		PrintStr("NO:");		
		sprintf(Data,"%02d",Dust.num);			
		PrintStrNnum(Data,2);	
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
		} 
		pprint(0x0d);  //回车
		pprint(0x0a); //换行
		pprint(0x0d);  //回车
		pprint(0x0a); //换行		 
}	
//排序函数并返回平均值  times:排序数据的个数 

u16 paixu(u8 times,u16 rcv_data[])
{
	u8  i, j; 
	u16 a;
	u32 b;

	for(i=0;i<times-1;i++)
	for(j=i+1;j<times;j++)
	if (rcv_data[i] <rcv_data[j])
			{
				a = rcv_data[i];
				rcv_data[i] = rcv_data[j];
				rcv_data[j] = a;
			}
	b=0;
	for(i=1; i<(times-1); i++)
	{
		b += rcv_data[i];
	}
	a=b/(times-2);
	return a;
}
#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
  while (1)
  {
  }
}
#endif


