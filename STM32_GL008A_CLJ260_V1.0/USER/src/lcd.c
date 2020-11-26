#include "lcd.h"
#include "stdlib.h"
#include "font.h"    
#include "string.h"
//////////////////////////////////////////////////////////////////////////////////	 
//7寸 TFT液晶驱动	  	
//********************************************************************************	 
				 
//LCD的画笔颜色和背景色	   
u16 POINT_COLOR=BLACK;	//画笔颜色
u16 BACK_COLOR=CYAN;	 //背景色,蓝绿色 

//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;
	
		   
//写寄存器函数
//regval:寄存器值
void LCD_WR_REG(u16 regval)
{ 
	LCD->LCD_REG=regval;//写入要写的寄存器序号	 
}
//写LCD数据
//data:要写入的值
void LCD_WR_DATA(u16 data)
{										    	   
	LCD->LCD_RAM=data;		 
}
//读LCD数据
//返回值:读到的值
u16 LCD_RD_DATA(void)
{										    	   
	return LCD->LCD_RAM;		 
}					   
//写寄存器
//LCD_Reg:寄存器地址
//LCD_RegValue:要写入的数据
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{	
	LCD->LCD_REG = LCD_Reg;		//写入要写的寄存器序号	 
	LCD->LCD_RAM = LCD_RegValue;//写入数据	    		 
}	   
//读寄存器
//LCD_Reg:寄存器地址
//返回值:读到的数据
u16 LCD_ReadReg(u8 LCD_Reg)
{										   
	LCD_WR_REG(LCD_Reg);		//写入要读的寄存器序号
	delay_us(5);		  
	return LCD_RD_DATA();		//返回读到的值
}   
//开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
 	LCD->LCD_REG=lcddev.wramcmd;	  
}	 
//LCD写GRAM
//RGB_Code:颜色值
void LCD_WriteRAM(u16 RGB_Code)
{							    
	LCD->LCD_RAM = RGB_Code;//写十六位GRAM
}	 
//读取个某点的颜色值	 
//x,y:坐标
//返回值:此点的颜色
u16 LCD_ReadPoint(u16 x,u16 y)
{
 	u16 r=0;
	if(x>=lcddev.width||y>=lcddev.height)return 0;	//超过了范围,直接返回		   
	LCD_SetCursor(x,y);	    
	
	LCD_WR_REG(0X2E);//9341/6804/3510/1963 发送读GRAM指令	 
 	r=LCD_RD_DATA();								//dummy Read	   
	return r;					//1963直接读就可以 	
}
//LCD背光设置
//pwm:背光等级,0~63.越大越亮.
void LCD_BackLightSet(u8 pwm)
{	
	LCD_WR_REG(0xBE);	//配置PWM输出
	LCD_WR_DATA(0x05);	//1设置PWM频率
	LCD_WR_DATA(pwm*2.55);//2设置PWM占空比
	LCD_WR_DATA(0x01);	//3设置C
	LCD_WR_DATA(0xFF);	//4设置D
	LCD_WR_DATA(0x00);	//5设置E
	LCD_WR_DATA(0x00);	//6设置F	   	 	 
}
//扫描方向上,X的终点坐标.
void LCD_EndXSet(u16 x)
{													    		
	LCD_WriteReg(LCD_END_X,x);				//设置X终坐标  	 	 
}			 
//LCD开启显示
void LCD_DisplayOn(void)
{	
	LCD_WR_REG(0X29);	//开启显示	   	 
}	 
//LCD关闭显示
void LCD_DisplayOff(void)
{	   
	LCD_WR_REG(0X28);	//关闭显示				//关闭TFT,相当于把背光关掉，无背光，无显示  	 
} 
//设置当前显示层
//layer:当前显示层 
void LCD_SetDisplayLayer(u16 layer)
{	 
	lcddev.sysreg&=~0X0E00;				   	//清除之前的设置
	lcddev.sysreg|=(layer&0X07)<<9;			//设置新的值
	LCD_WriteReg(LCD_PREF,lcddev.sysreg);	//写LCD_PREF寄存器	   	 	 
} 
//设置当前操作层
//layer:当前显示层 
void LCD_SetOperateLayer(u16 layer)
{	 
	lcddev.sysreg&=~0X7000;					//清除之前的设置
	lcddev.sysreg|=(layer&0X07)<<12;		//设置新的值
	LCD_WriteReg(LCD_PREF,lcddev.sysreg);	//写LCD_PREF寄存器	   	 	 
} 	     
//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	 
	if(lcddev.dir==0)//x坐标需要变换
	{
		Xpos=lcddev.width-1-Xpos;
		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(0);LCD_WR_DATA(0); 		
		LCD_WR_DATA(Xpos>>8);LCD_WR_DATA(Xpos&0XFF);		 	 
	}
	else
	{
		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(Xpos>>8);LCD_WR_DATA(Xpos&0XFF); 		
		LCD_WR_DATA((lcddev.width-1)>>8);LCD_WR_DATA((lcddev.width-1)&0XFF);		 	 			
	}	
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA(Ypos>>8);LCD_WR_DATA(Ypos&0XFF); 		
	LCD_WR_DATA((lcddev.height-1)>>8);LCD_WR_DATA((lcddev.height-1)&0XFF);	 
} 		 										 
//设置LCD的自动扫描方向
//注意:我们的驱动器,只支持左右扫描设置,不支持上下扫描设置   	   
void LCD_Scan_Dir(u8 dir)
{			   
  u16 regval=0;
	u16 dirreg=0;
	
	if((lcddev.dir==1&&lcddev.id!=0X1963)||(lcddev.dir==0&&lcddev.id==0X1963))//横屏时，对6804和1963不改变扫描方向！竖屏时1963改变方向
	{			   
		switch(dir)//方向转换
		{
			case 0:dir=6;break;
			case 1:dir=7;break;
			case 2:dir=4;break;
			case 3:dir=5;break;
			case 4:dir=1;break;
			case 5:dir=0;break;
			case 6:dir=3;break;
			case 7:dir=2;break;	     
		}
	} 
	if(lcddev.id==0X1963) 
	{  
    switch(dir)
	{
		case L2R_U2D://从左到右,从上到下
			regval|=(0<<7)|(0<<6)|(0<<5); 
			break;
		case L2R_D2U://从左到右,从下到上
			regval|=(1<<7)|(0<<6)|(0<<5); 
			break;
		case R2L_U2D://从右到左,从上到下
			regval|=(0<<7)|(1<<6)|(0<<5); 
			break;
		case R2L_D2U://从右到左,从下到上
			regval|=(1<<7)|(1<<6)|(0<<5); 
			break;	 
		case U2D_L2R://从上到下,从左到右
			regval|=(0<<7)|(0<<6)|(1<<5); 
			break;
		case U2D_R2L://从上到下,从右到左
			regval|=(0<<7)|(1<<6)|(1<<5); 
			break;
		case D2U_L2R://从下到上,从左到右
			regval|=(1<<7)|(0<<6)|(1<<5); 
			break;
		case D2U_R2L://从下到上,从右到左
			regval|=(1<<7)|(1<<6)|(1<<5); 
			break; 
	}
	dirreg=0X36;
	LCD_WriteReg(dirreg,regval);
	LCD_WR_REG(lcddev.setxcmd); 
	LCD_WR_DATA(0);
	LCD_WR_DATA(0);
	LCD_WR_DATA((lcddev.width-1)>>8);
	LCD_WR_DATA((lcddev.width-1)&0XFF);
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA(0);
	LCD_WR_DATA(0);
	LCD_WR_DATA((lcddev.height-1)>>8);
	LCD_WR_DATA((lcddev.height-1)&0XFF); 
  }
}   
//画点
//x,y:坐标
//POINT_COLOR:此点的颜色
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);		//设置光标位置 
	LCD_WriteRAM_Prepare();	//开始写入GRAM
	LCD->LCD_RAM=POINT_COLOR; 
}
//画点
//x,y:坐标
//POINT_COLOR:此点的颜色
void GUI_Point(u16 x,u16 y,u16 color)
{
	LCD_SetCursor(x,y);		//设置光标位置 
	LCD_WriteRAM_Prepare();	//开始写入GRAM
	LCD->LCD_RAM=color; 
}
//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint(u16 x,u16 y,u16 color)
{	  		 		   
	LCD->LCD_REG=lcddev.setycmd;
	LCD->LCD_RAM=y; 
 	LCD->LCD_REG=lcddev.setxcmd; 
	LCD->LCD_RAM=x; 
	LCD->LCD_REG=lcddev.wramcmd; 
	LCD->LCD_RAM=color; 
}	
//设置LCD显示方向（7寸屏,不能简单的修改为横屏显示）
//dir:0,竖屏；1,横屏
void LCD_Display_Dir(u8 dir)
{
	if(dir==0)			//竖屏
	{
	lcddev.dir=0;	//竖屏
	lcddev.wramcmd=0X2C;	//设置写入GRAM的指令 
	lcddev.setxcmd=0X2B;	//设置写X坐标指令
	lcddev.setycmd=0X2A;	//设置写Y坐标指令
	lcddev.width=480;		//设置宽度480
	lcddev.height=800;		//设置高度800  
		
	}
	else 				//横屏
	{  				
	lcddev.dir=1;	//横屏	
	lcddev.wramcmd=0X2C;	//设置写入GRAM的指令 
	lcddev.setxcmd=0X2A;	//设置写X坐标指令
	lcddev.setycmd=0X2B;	//设置写Y坐标指令
	lcddev.width=800;		//设置宽度800
	lcddev.height=480;		//设置高度480  	
		
	} 
	LCD_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向
}
//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).
//sx,sy:窗口起始坐标(左上角)
//width,height:窗口宽度和高度,必须大于0!!
//窗体大小:width*height.
//68042,横屏时不支持窗口设置!! 
void LCD_Set_Window(u16 sx,u16 sy,u16 width,u16 height)
{   			    
	u16 twidth,theight;
	twidth=sx+width-1;
	theight=sy+height-1;			    
		
	if(lcddev.dir==1&&lcddev.id==0X1963)
	{
		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(sx>>8); 
		LCD_WR_DATA(sx&0XFF);	 
		LCD_WR_DATA(twidth>>8); 
		LCD_WR_DATA(twidth&0XFF);  
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(sy>>8); 
		LCD_WR_DATA(sy&0XFF); 
		LCD_WR_DATA(theight>>8); 
		LCD_WR_DATA(theight&0XFF);
	}
	else if(lcddev.id==0X1963)//1963竖屏特殊处理
	{	
	sx=lcddev.width-width-sx; 
	height=sy+height-1; 
	LCD_WR_REG(lcddev.setxcmd); 
	LCD_WR_DATA(sx>>8); 
	LCD_WR_DATA(sx&0XFF);	 
	LCD_WR_DATA((sx+width-1)>>8); 
	LCD_WR_DATA((sx+width-1)&0XFF);  
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA(sy>>8); 
	LCD_WR_DATA(sy&0XFF); 
	LCD_WR_DATA(height>>8); 
	LCD_WR_DATA(height&0XFF); 
	}
}
//初始化lcd
//该初始化函数可以初始化各种ILI93XX液晶,但是其他函数是基于ILI9320的!!!
//在其他型号的驱动芯片上没有测试! 
void LCD_Init(void)
{ 
	delay_ms(10);	
	LCD_WR_REG(0XA1);
	lcddev.id=LCD_RD_DATA();
	lcddev.id=LCD_RD_DATA();	//读回0X57
	lcddev.id<<=8;	 
	lcddev.id|=LCD_RD_DATA();	//读回0X61	
	if(lcddev.id==0X5761)lcddev.id=0X1963;//SSD1963读回的ID是5761H,为方便区分,我们强制设置为1963
	else lcddev.id=0X0570;//现用触摸屏V1.8或v2.3版本，v2.3为1963驱动ic，v1.8为EPM570T cpld;
	if(lcddev.id==0X1963)	//1963初始化
	{
	LCD_WR_REG(0xE2);		//Set PLL with OSC = 10MHz (hardware),	Multiplier N = 35, 250MHz < VCO < 800MHz = OSC*(N+1), VCO = 300MHz
	LCD_WR_DATA(0x1D);		//参数1 
	LCD_WR_DATA(0x02);		//参数2 Divider M = 2, PLL = 300/(M+1) = 100MHz
	LCD_WR_DATA(0x04);		//参数3 Validate M and N values   
	delay_us(10);
	LCD_WR_REG(0xE0);		// Start PLL command
	LCD_WR_DATA(0x01);		// enable PLL
	delay_us(1);
	LCD_WR_REG(0xE0);		// Start PLL command again
	LCD_WR_DATA(0x03);		// now, use PLL output as system clock	
	delay_us(10);  
	LCD_WR_REG(0x01);		//软复位
	delay_us(10);
	
	LCD_WR_REG(0xE6);		//设置像素频率,33Mhz
	LCD_WR_DATA(0x2F);
	LCD_WR_DATA(0xFF);
	LCD_WR_DATA(0xFF);
	
	LCD_WR_REG(0xB0);		//设置LCD模式
	LCD_WR_DATA(0x20);		//24位模式
	LCD_WR_DATA(0x00);		//TFT 模式 

	LCD_WR_DATA((SSD_HOR_RESOLUTION-1)>>8);//设置LCD水平像素
	LCD_WR_DATA(SSD_HOR_RESOLUTION-1);		 
	LCD_WR_DATA((SSD_VER_RESOLUTION-1)>>8);//设置LCD垂直像素
	LCD_WR_DATA(SSD_VER_RESOLUTION-1);		 
	LCD_WR_DATA(0x00);		//RGB序列 
	
	LCD_WR_REG(0xB4);		//Set horizontal period
	LCD_WR_DATA((SSD_HT-1)>>8);
	LCD_WR_DATA(SSD_HT-1);
	LCD_WR_DATA(SSD_HPS>>8);
	LCD_WR_DATA(SSD_HPS);
	LCD_WR_DATA(SSD_HOR_PULSE_WIDTH-1);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_REG(0xB6);		//Set vertical period
	LCD_WR_DATA((SSD_VT-1)>>8);
	LCD_WR_DATA(SSD_VT-1);
	LCD_WR_DATA(SSD_VPS>>8);
	LCD_WR_DATA(SSD_VPS);
	LCD_WR_DATA(SSD_VER_FRONT_PORCH-1);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	
	LCD_WR_REG(0xF0);	//设置SSD1963与CPU接口为16bit  
	LCD_WR_DATA(0x03);	//16-bit(565 format) data for 16bpp 

	LCD_WR_REG(0x29);	//开启显示
	//设置PWM输出  背光通过占空比可调 
	LCD_WR_REG(0xD0);	//设置自动白平衡DBC
	LCD_WR_DATA(0x00);	//disable

	LCD_WR_REG(0xBE);	//配置PWM输出
	LCD_WR_DATA(0x05);	//1设置PWM频率
	LCD_WR_DATA(0xFE);	//2设置PWM占空比
	LCD_WR_DATA(0x01);	//3设置C
	LCD_WR_DATA(0x00);	//4设置D
	LCD_WR_DATA(0x00);	//5设置E 
	LCD_WR_DATA(0x00);	//6设置F 
	
	LCD_WR_REG(0xB8);	//设置GPIO配置
	LCD_WR_DATA(0x03);	//2个IO口设置成输出
	LCD_WR_DATA(0x01);	//GPIO使用正常的IO功能 
	LCD_WR_REG(0xBA);
	LCD_WR_DATA(0X01);	//GPIO[1:0]=01,控制LCD方向
	
	LCD_BackLightSet(100);//背光设置为最亮

	LCD_Display_Dir(1);		//默认为竖屏  0:竖屏 1:横屏
	
	LCD_Clear(CYAN);
} 
}
//清屏函数
//color:要清屏的填充色
void LCD_Clear(u16 color)
{
	u32 index=0;      
	u32 totalpoint = 384000;//800*480
	LCD_SetCursor(0x00,0x0000);	//设置光标位置
	LCD_WriteRAM_Prepare();     //开始写入GRAM 	  
	for(index=0;index<totalpoint;index++)
	{
		LCD->LCD_RAM=color;	   
	}
}  
//在指定区域内填充单个颜色
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{          
	u16 i,j;
	u16 xlen=0;
	xlen=ex-sx+1;	   
	for(i=sy;i<=ey;i++)
	{
	 	LCD_SetCursor(sx,i);      				//设置光标位置 
		LCD_WriteRAM_Prepare();     			//开始写入GRAM	  
		for(j=0;j<xlen;j++)LCD->LCD_RAM=color;	//送入LCD	    
	}
}  
//在指定区域内填充指定颜色块			 
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color)
{  
	u16 height,width;
	u16 i,j;
	width=ex-sx+1; 		//得到填充的宽度
	height=ey-sy+1;		//高度
 	for(i=0;i<height;i++)
	{
 		LCD_SetCursor(sx,sy+i);   	//设置光标位置 
		LCD_WriteRAM_Prepare();     //开始写入GRAM
		for(j=0;j<width;j++)LCD->LCD_RAM=color[i*height+j];//写入数据 
	}	  
}  
//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_DrawPoint(uRow,uCol);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}
//在三角形区域填充指定颜色 
void TriangularRegion(u16 x1, u16 x2, u16 y1, u16 x3, u16 y3, u16 color)
{
	u16 i;
	POINT_COLOR = color;
	for(i=x1; i<x2; i++)
	{
		LCD_DrawLine(x1, y1, x3, y3);
		x1++;
	}
}   
//画矩形	  
//(x1,y1),(x2,y2):矩形的对角坐标
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void Draw_Circle(u16 x0,u16 y0,u8 r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		LCD_DrawPoint(x0+a,y0-b);             //5
 		LCD_DrawPoint(x0+b,y0-a);             //0           
		LCD_DrawPoint(x0+b,y0+a);             //4               
		LCD_DrawPoint(x0+a,y0+b);             //6 
		LCD_DrawPoint(x0-a,y0+b);             //1       
 		LCD_DrawPoint(x0-b,y0+a);             
		LCD_DrawPoint(x0-a,y0-b);             //2             
  		LCD_DrawPoint(x0-b,y0-a);             //7     	         
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
} 

//画一条粗线
//(x1,y1),(x2,y2):线条的起始坐标
//size：线条的粗细程度
//color：线条的颜色
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		gui_fill_circle(uRow,uCol,size,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
} 

//画实心圆
//x0,y0:坐标
//r:半径
//color:颜色
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{											  
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
} 

//画水平线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}

//画垂直线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_vline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0,y0+len-1,color);	
}
									  
//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{  							  
    u8 temp,t1,t,size1;
	u16 y0=y;
	u16 colortemp=POINT_COLOR;      			     
	//设置窗口		   
	num=num-' ';//得到偏移后的值
	if(size == 12||size == 16) size1 = size;
	else 
		if(size == 24) size1 = 36;
		else size1 = size*2;
	if(!mode) //非叠加方式
	{
	    for(t=0; t<size1; t++)
	    {   
			if(size==16) temp=asc2_1608[num][t];		 //调用1608字体 	                          
	    else if(size == 24) temp=asc2_2412[num][t];
		  else temp=asc2_3216[num][t];		 //调用3216字体 	                          
	        for(t1=0; t1<8; t1++)
			{			    
		        if(temp&0x80)POINT_COLOR=colortemp;
				else POINT_COLOR=BACK_COLOR;
				LCD_DrawPoint(x,y);	
				temp<<=1;
				y++;
				if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超区域了
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超区域了
					break;
				}
			}  	 
	    }    
	}
	else//叠加方式
	{
	    for(t=0; t<size1; t++)
	    {   
			if(size==16)temp=asc2_1608[num][t];		 //调用1608字体
				else if(size==24) temp=asc2_2412[num][t];
					else temp=asc2_3216[num][t];       
	        for(t1=0; t1<8; t1++)
			{			    
		        if(temp&0x80)LCD_DrawPoint(x,y); 
				temp<<=1;
				y++;
				if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超区域了
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超区域了
					break;
				}
			}  	 
	    }     
	}
	POINT_COLOR=colortemp;	    	   	 	  
}   
//m^n函数
//返回值:m^n次方.
u32 LCD_Pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}			 
//显示数字,高位为0,则不显示
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//color:颜色 
//num:数值(0~4294967295);	 
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
	}
} 
//显示数字,高位为0,还是显示
//x,y:起点坐标
//num:数值(0~999999999);	 
//len:长度(即要显示的位数)
//size:字体大小
//mode:
//[7]:0,不填充;1,填充0.
//[6:1]:保留
//[0]:0,非叠加显示;1,叠加显示.
void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode)
{  
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				if(mode&0X80)LCD_ShowChar(x+(size/2)*t,y,'0',size,mode&0X01);  
				else LCD_ShowChar(x+(size/2)*t,y,' ',size,mode&0X01);  
 				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,mode&0X01); 
	}
} 
//显示字符串
//x,y:起点坐标
//width,height:区域大小  
//size:字体大小
//*p:字符串起始地址		  
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p)
{         
	u8 x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//退出
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }  
}

/********************************************/
//放置向上箭头	 /\(x,y)
//				/  \
//			 1 /	\2
//	   (x1,y1)/_3  _4\(x2,y1)
//		 (x3,y1)5| |(x4,y1)
//			     |_|6  
//		   (x3,y2)7 (x4,y2)
/*******************************************/
void JianTou_Up(u16 x, u16 y, u8 size, u16 color)
{
  	u16 x1, x2, x3, x4, y1, y2;
	x1 = x - 45;
	x2 = x + 45;
	x3 = x - 10;
	x4 = x + 10;
	y1 = y + 40;
	y2 = y + 80;
	lcd_draw_bline(x, y, x1, y1, size, color);	//画线1 (x,y):起点坐标,(x1,y1):终点坐标
	lcd_draw_bline(x, y, x2, y1, size, color);	//画线2 (x,y):起点坐标,(x2,y1):终点坐标
	lcd_draw_bline(x1, y1, x3, y1, size, color);//画线3 (x1,y1):起点坐标,(x3,y1):终点坐标
	lcd_draw_bline(x4, y1, x2, y1, size, color);//画线4 (x4,y1):起点坐标,(x2,y1):终点坐标
	lcd_draw_bline(x3, y1, x3, y2, size, color);//画线5 (x3,y1):起点坐标,(x3,y2):终点坐标
	lcd_draw_bline(x4, y1, x4, y2, size, color);//画线6 (x4,y1):起点坐标,(x4,y2):终点坐标
	lcd_draw_bline(x3, y2, x4, y2, size, color);//画线7 (x3,y2):起点坐标,(x4,y2):终点坐标	
}

/********************************************/
//放置向下箭头(x3,y2)__7_ (x4,y2)
//			(x3,y1) 5|	|6 (x4,y1)
//			     3 __|  |__4
//		   (x1,y1) \      /(x2,y1)
//			    	\    /
//			 	   1 \  /2
//		  )			  \/(x,y)
/*******************************************/
void JianTou_Down(u16 x, u16 y, u8 size, u16 color)
{
  	u16 x1, x2, x3, x4, y1, y2;
	x1 = x - 45;
	x2 = x + 45;
	x3 = x - 10;
	x4 = x + 10;
	y1 = y - 40;
	y2 = y - 80;
	lcd_draw_bline(x, y, x1, y1, size, color);	//画线1 (x,y):起点坐标,(x1,y1):终点坐标
	lcd_draw_bline(x, y, x2, y1, size, color);	//画线2 (x,y):起点坐标,(x2,y1):终点坐标
	lcd_draw_bline(x1, y1, x3, y1, size, color);//画线3 (x1,y1):起点坐标,(x3,y1):终点坐标
	lcd_draw_bline(x4, y1, x2, y1, size, color);//画线4 (x4,y1):起点坐标,(x2,y1):终点坐标
	lcd_draw_bline(x3, y1, x3, y2, size, color);//画线5 (x3,y1):起点坐标,(x3,y2):终点坐标
	lcd_draw_bline(x4, y1, x4, y2, size, color);//画线6 (x4,y1):起点坐标,(x4,y2):终点坐标
	lcd_draw_bline(x3, y2, x4, y2, size, color);//画线7 (x3,y2):起点坐标,(x4,y2):终点坐标	
}



void Show_GB16(u16 x,u16 y,u8 *font,u8 size,u8 mode)
{
	u8 temp,t,t1,k;
	u16 y0=y;
	u16 tempcolor=POINT_COLOR;;
	if(mode==0)
	{	
		for(k=0; k<100; k++) 
		{
			if((codeGB_16[k].Index[0]==font[0])&&(codeGB_16[k].Index[1]==font[1]))
		    {		
				for(t=0;t<32;t++)
			    {   												   
				    temp=codeGB_16[k].Msk[t];//μ?μ?12êy?Y                          
			        for(t1=0;t1<8;t1++)
					{
						if(temp&0x80)LCD_DrawPoint(x,y);
			 			else 
						{
							tempcolor=POINT_COLOR;
							POINT_COLOR=BACK_COLOR;
							LCD_DrawPoint(x,y);
							POINT_COLOR=tempcolor;//?1?-
						}
						temp<<=1;
						y++;
						if((y-y0)==size)
						{
							y=y0;
							x++;
							break;
						}
					}
				} 
				break;
	    	}
		} 
	}
	else//μt?ó??ê?
	{
		
	   for(k=0; k<100; k++)
	   {
	   		if((codeGB_16[k].Index[0]==font[0])&&(codeGB_16[k].Index[1]==font[1]))
			{
			    for(t=0;t<32;t++)
			    {   												   
				    temp=codeGB_16[k].Msk[t];//μ?μ?12êy?Y                          
			        for(t1=0;t1<8;t1++)
					{
						if(temp&0x80)LCD_DrawPoint(x,y);   
						temp<<=1;
						y++;
						if((y-y0)==size)
						{
							y=y0;
							x++;
							break;
						}
					}  	 
		    	}
				break;
			}
		}
	}    
}
//显示一个指定大小的汉字
//X,Y汉字的坐标
//font 汉字GBK码
//size 字体大小 mode 0正常显示 1 叠加显示
void Show_GB24(u16 x,u16 y,u8 *font,u8 size,u8 mode)
{
	u8 temp,t,t1,k;
	u16 y0=y;
	u16 tempcolor=POINT_COLOR;;
	if(mode==0)//?y3￡??ê?
	{	
		for(k=0; k<200; k++) 
		{
			if((codeGB_24[k].Index[0]==font[0])&&(codeGB_24[k].Index[1]==font[1]))
		    {		
				for(t=0;t<72;t++)//GB24  72
			    {   												   
				    temp=codeGB_24[k].Msk[t];//μ?μ?12êy?Y                          
			        for(t1=0;t1<8;t1++)
					{
						if(temp&0x80)LCD_DrawPoint(x,y);
			 			else 
						{
							tempcolor=POINT_COLOR;
							POINT_COLOR=BACK_COLOR;
							LCD_DrawPoint(x,y);
							POINT_COLOR=tempcolor;//?1?-
						}
						temp<<=1;
						y++;
						if((y-y0)==size)
						{
							y=y0;
							x++;
							break;
						}
					}
				} 
				break;
	    	}
		} 
	}
	else//μt?ó??ê?
	{
		
	   for(k=0; k<200; k++)
	   {
	   		if((codeGB_24[k].Index[0]==font[0])&&(codeGB_24[k].Index[1]==font[1]))
			{
			    for(t=0;t<72;t++)
			    {   												   
				    temp=codeGB_24[k].Msk[t];//μ?μ?12êy?Y                          
			        for(t1=0;t1<8;t1++)
					{
						if(temp&0x80)LCD_DrawPoint(x,y);   
						temp<<=1;
						y++;
						if((y-y0)==size)
						{
							y=y0;
							x++;
							break;
						}
					}  	 
		    	}
				break;
			}
		}
	}    
}
//显示一个指定大小的汉字
//x,y :汉字的坐标
//font:汉字GBK码
//size:字体大小
//mode:0,正常显示,1,叠加显示	   
void Show_GB32(u16 x,u16 y,u8 *font,u8 size,u8 mode)
{
	u8 temp,t,t1,k;
	u16 y0=y;
//	u8 dzk[32];
	u16 tempcolor;	  
//	if(size!=12&&size!=16)return;//不支持的size
//	Get_HzMat(font,dzk,size);//得到相应大小的点阵数据
	if(mode==0)//正常显示
	{	
		for(k=0; k<200; k++) 
		{
			if((codeGB_32[k].Index[0]==font[0])&&(codeGB_32[k].Index[1]==font[1]))
		    {
		//		for(t=0;t<size*2;t++)
				for(t=0;t<size*4;t++)//GB32x4
			    {   												   
				    temp=codeGB_32[k].Msk[t];//得到12数据                          
			        for(t1=0;t1<8;t1++)
					{
						if(temp&0x80)LCD_DrawPoint(x,y);
			 			else 
						{
							tempcolor=POINT_COLOR;
							POINT_COLOR=BACK_COLOR;
							LCD_DrawPoint(x,y);
							POINT_COLOR=tempcolor;//还原
						}
						temp<<=1;
						y++;
						if((y-y0)==size)
						{
							y=y0;
							x++;
							break;
						}
					}
				} 
				break;
	    	}
		} 
	}
	else//叠加显示
	{
	   for(k=0; k<200; k++)
	   {
	   		if((codeGB_32[k].Index[0]==font[0])&&(codeGB_32[k].Index[1]==font[1]))
			{
			    for(t=0;t<size*4;t++)
			    {   												   
				    temp=codeGB_32[k].Msk[t];//得到12数据                          
			        for(t1=0;t1<8;t1++)
					{
						if(temp&0x80)LCD_DrawPoint(x,y);   
						temp<<=1;
						y++;
						if((y-y0)==size)
						{
							y=y0;
							x++;
							break;
						}
					}  	 
		    	}
				break;
			}
		}
	}    
}




//在指定位置开始显示一个字符串	    
//支持自动换行
//(x,y):起始坐标
//width,height:区域
//str  :字符串
//size :字体大小
//mode:0,非叠加方式;1,叠加方式    	   		   
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode)
{					
	u16 x0=x;
	u16 y0=y;							  	  
    u8 bHz=0;     //字符或者中文  	    				    				  	  
    while(*str!=0)//数据未结束
    { 
        if(!bHz)
        {
	        if(*str>0x80)bHz=1;//中文 
	        else              //字符
	        {      
                if(x>(x0+width-size/2))//换行
				{				   
					y+=size;
					x=x0;	   
				}							    
		        if(y>(y0+height-size))break;//越界返回      
		        if(*str==13)//换行符号
		        {         
		            y+=size;
					x=x0;
		            str++; 
		        }  
		        else LCD_ShowChar(x,y,*str,size,mode);//有效部分写入 
				str++; 
		        x+=size/2; //字符,为全字的一半 
	        }
        }
				else//中文 
        {     
            bHz=0;//有汉字库 
            if(x>(x0+width-size))//换行
			{	    
				y+=size;
				x=x0;		  
			}
	    if(y>(y0+height-size))break;//越界返回
			if(size==16)
				Show_GB16(x,y,str,size,mode);	
			else if(size==24)
				Show_GB24(x,y,str,size,mode);
			else if(size==32)	 
				Show_GB32(x,y,str,size,mode);
	        str+=2; 
	        x+=size;//下一个汉字偏移	    
        }						 
    }   
}  			 		 
//在指定宽度的中间显示字符串
//如果字符长度超过了len,则用Show_Str显示
//len:指定要显示的宽度			  
void Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u8 len)
{
	u16 strlenth=0;
   	strlenth=strlen((const char*)str);
	strlenth*=size/2;
	if(strlenth>len)Show_Str(x,y,240,320,str,size,1);
	else
	{	strlenth=(len-strlenth)/2;
	    Show_Str(strlenth+x,y,240,320,str,size,1);
	}
}

u16 GU16_key_color = LIGHTGRAY;		//按钮表面颜色

u16 GU16_bordercolor_light = WHITE;	//按键边框浅色部分颜色
u16 GU16_bordercolor_dark = LGRAY;	//按键边框深色部分颜色

/********************************************************************/
//函数功能：放置按键
//(x,y)矩形按键中心坐标
//len:按键长度
//wid:按键宽度
/********************************************************************/
void Put_AnJian(u16 x, u16 y, u16 len, u16 wid)
{
	u16 lx_inside, ly_inside, rx_inside, ry_inside;
	lx_inside = x - len/2;
	ly_inside = y - wid/2;
	rx_inside = x + len/2;
	ry_inside = y + wid/2;
	LCD_Fill(lx_inside,ly_inside,rx_inside,ry_inside,GU16_key_color); 						//按键主颜色

	LCD_Fill(lx_inside-1, ly_inside-1, lx_inside-1, ry_inside+1, GU16_bordercolor_light);	//画左垂直边框线,汪色
	LCD_Fill(lx_inside-2, ly_inside-2, lx_inside-2, ry_inside+2, GU16_bordercolor_light);
	LCD_Fill(lx_inside-3, ly_inside-3, lx_inside-3, ry_inside+3, GU16_bordercolor_light);
	
	LCD_Fill(lx_inside-1, ly_inside-1, rx_inside+1, ly_inside-1, GU16_bordercolor_light);	//画上边框线,浅色
	LCD_Fill(lx_inside-2, ly_inside-2, rx_inside+2, ly_inside-2, GU16_bordercolor_light);
	LCD_Fill(lx_inside-3, ly_inside-3, rx_inside+3, ly_inside-3, GU16_bordercolor_light);	

	LCD_Fill(rx_inside+1, ly_inside,   rx_inside+1, ry_inside+1, GU16_bordercolor_dark);		//画右垂直边框线,深色
	LCD_Fill(rx_inside+2, ly_inside-1, rx_inside+2, ry_inside+2, GU16_bordercolor_dark);
	LCD_Fill(rx_inside+3, ly_inside-2, rx_inside+3, ry_inside+3, GU16_bordercolor_dark);
	
	LCD_Fill(lx_inside-1, ry_inside,   rx_inside+1, ry_inside-1, GU16_bordercolor_dark);		//画下边框线,深色
	LCD_Fill(lx_inside-2, ry_inside+1, rx_inside+2, ry_inside+2, GU16_bordercolor_dark);
	LCD_Fill(lx_inside-3, ry_inside+2, rx_inside+3, ry_inside+3, GU16_bordercolor_dark);	
	if (len > 50) //当按键长度大于50时，边框线变宽
	{
		LCD_Fill(lx_inside-4, ly_inside-4, lx_inside-4, ry_inside+4, GU16_bordercolor_light); //画左垂直边框线,浅色
		LCD_Fill(lx_inside-5, ly_inside-5, lx_inside-5, ry_inside+5, GU16_bordercolor_light);
		LCD_Fill(lx_inside-6, ly_inside-6, lx_inside-6, ry_inside+6, GU16_bordercolor_light);
	
		LCD_Fill(lx_inside-4, ly_inside-4, rx_inside+4, ly_inside-4, GU16_bordercolor_light); //画上边框线,浅色
		LCD_Fill(lx_inside-5, ly_inside-5, rx_inside+5, ly_inside-5, GU16_bordercolor_light);
		LCD_Fill(lx_inside-6, ly_inside-6, rx_inside+6, ly_inside-6, GU16_bordercolor_light);
	
		LCD_Fill(rx_inside+4, ly_inside-3, rx_inside+4, ry_inside+4, GU16_bordercolor_dark);	//画右垂直边框线,深色
		LCD_Fill(rx_inside+5, ly_inside-4, rx_inside+5, ry_inside+5, GU16_bordercolor_dark);
		LCD_Fill(rx_inside+6, ly_inside-5, rx_inside+6, ry_inside+6, GU16_bordercolor_dark);
	
		LCD_Fill(lx_inside-4, ry_inside+3, rx_inside+4, ry_inside+4, GU16_bordercolor_dark);	 //画下边框线,深色
		LCD_Fill(lx_inside-5, ry_inside+4, rx_inside+5, ry_inside+5, GU16_bordercolor_dark);
		LCD_Fill(lx_inside-6, ry_inside+5, rx_inside+6, ry_inside+6, GU16_bordercolor_dark);
	}	
}

/********************************************************************/
//按键被按下时的状态
//(x,y)矩形按键中心坐标
//len:按键长度
//wid:按键宽度
/********************************************************************/
void AnJianPressed(u16 x, u16 y, u16 len, u16 wid)
{
	u16 lx_inside, ly_inside, rx_inside, ry_inside;
	lx_inside = x - len/2;
	ly_inside = y - wid/2;
	rx_inside = x + len/2;
	ry_inside = y + wid/2;
	LCD_Fill(lx_inside,ly_inside,rx_inside,ry_inside,GU16_key_color); 						//按键主颜色

	LCD_Fill(lx_inside-1, ly_inside-1, lx_inside-1, ry_inside+1, GU16_bordercolor_dark);	//画左垂直边框线
	LCD_Fill(lx_inside-2, ly_inside-2, lx_inside-2, ry_inside+2, GU16_bordercolor_dark);
	LCD_Fill(lx_inside-3, ly_inside-3, lx_inside-3, ry_inside+3, GU16_bordercolor_dark);

	LCD_Fill(lx_inside-1, ly_inside-1, rx_inside+1, ly_inside-1, GU16_bordercolor_dark);	//画上边框线
	LCD_Fill(lx_inside-2, ly_inside-2, rx_inside+2, ly_inside-2, GU16_bordercolor_dark);
	LCD_Fill(lx_inside-3, ly_inside-3, rx_inside+3, ly_inside-3, GU16_bordercolor_dark);
	
	LCD_Fill(rx_inside+1, ly_inside,   rx_inside+1, ry_inside+1, GU16_bordercolor_light);	//画右垂直边框线
	LCD_Fill(rx_inside+2, ly_inside-1, rx_inside+2, ry_inside+2, GU16_bordercolor_light);
	LCD_Fill(rx_inside+3, ly_inside-2, rx_inside+3, ry_inside+3, GU16_bordercolor_light);
	
	LCD_Fill(lx_inside-1, ry_inside,   rx_inside+1, ry_inside-1, GU16_bordercolor_light);	//画下边框线
	LCD_Fill(lx_inside-2, ry_inside+1, rx_inside+2, ry_inside+2, GU16_bordercolor_light);
	LCD_Fill(lx_inside-3, ry_inside+2, rx_inside+3, ry_inside+3, GU16_bordercolor_light);
	if (len > 50)  //当按键长度大于50时，边框线变宽
	{
		LCD_Fill(lx_inside-4, ly_inside-4, lx_inside-4, ry_inside+4, GU16_bordercolor_dark);	//画左垂直边框线
		LCD_Fill(lx_inside-5, ly_inside-5, lx_inside-5, ry_inside+5, GU16_bordercolor_dark);
		LCD_Fill(lx_inside-6, ly_inside-6, lx_inside-6, ry_inside+6, GU16_bordercolor_dark);

		LCD_Fill(lx_inside-4, ly_inside-4, rx_inside+4, ly_inside-4, GU16_bordercolor_dark);	//画上边框线
		LCD_Fill(lx_inside-5, ly_inside-5, rx_inside+5, ly_inside-5, GU16_bordercolor_dark);
		LCD_Fill(lx_inside-6, ly_inside-6, rx_inside+6, ly_inside-6, GU16_bordercolor_dark);

		LCD_Fill(rx_inside+4, ly_inside-3, rx_inside+4, ry_inside+4, GU16_bordercolor_light); //画右垂直边框线
		LCD_Fill(rx_inside+5, ly_inside-4, rx_inside+5, ry_inside+5, GU16_bordercolor_light);
		LCD_Fill(rx_inside+6, ly_inside-5, rx_inside+6, ry_inside+6, GU16_bordercolor_light);

		LCD_Fill(lx_inside-4, ry_inside+3, rx_inside+4, ry_inside+4, GU16_bordercolor_light); //画下边框线
		LCD_Fill(lx_inside-5, ry_inside+4, rx_inside+5, ry_inside+5, GU16_bordercolor_light);
		LCD_Fill(lx_inside-6, ry_inside+5, rx_inside+6, ry_inside+6, GU16_bordercolor_light); 	
	}		
}
/* 定义十进制(0-7)==>十六进制位转换表，由于显示点数据是由左到右，所以十六进制位顺序是倒的 */
u8 const  DCB2HEX_TAB[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
u16  disp_color;
u16	 back_color;

u8  GUI_LoadLine(u32 x, u32 y, u8 *dat, u32 no)
{  u8   bit_dat;  
   u16   i;
   u16  bakc;

   /* 参数过滤 */
   if(x>=800) return(0);
   if(y>=480) return(0);
   
   for(i=0; i<no; i++)
   {  /* 判断是否要读取点阵数据 */
      if( (i%8)==0 ) bit_dat = *dat++;
     
      /* 设置相应的点为color或为back_color */
      if( (bit_dat&DCB2HEX_TAB[i&0x07])==0 ) GUI_CopyColor(&bakc, back_color); 
         else  GUI_CopyColor(&bakc, disp_color);
      GUI_Point(x, y, bakc);       
     
      if( (++x)>=800 ) return(0);
   }
   
   return(1);
}

/****************************************************************************
* 名称：GUI_LoadPic()
* 功能：输出单色图形数据。
* 入口参数： x		指定显示位置，x坐标(x必须是8的整数倍)
*           y		指定显示位置，y坐标
*           dat		要输出显示的数据
*           hno     要显示此行的点个数
*           lno     要显示此列的点个数
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  GUI_LoadPic(u32 x, u32 y, u8 *dat, u32 hno, u32 lno)
{  u32  i; 
   for(i=0; i<lno; i++)
   {  GUI_LoadLine(x, y, dat, hno);				// 输出一行数据
      y++;										// 显示下一行
      dat += (hno>>3);							// 计算下一行的数据
      if( (hno&0x07)!=0 ) dat++;
   }
}

void  GUI_SetColor(u16 color1, u16 color2)
{  GUI_CopyColor(&disp_color, color1);
   GUI_CopyColor(&back_color, color2);  
}




