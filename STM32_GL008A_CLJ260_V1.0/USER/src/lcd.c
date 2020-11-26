#include "lcd.h"
#include "stdlib.h"
#include "font.h"    
#include "string.h"
//////////////////////////////////////////////////////////////////////////////////	 
//7�� TFTҺ������	  	
//********************************************************************************	 
				 
//LCD�Ļ�����ɫ�ͱ���ɫ	   
u16 POINT_COLOR=BLACK;	//������ɫ
u16 BACK_COLOR=CYAN;	 //����ɫ,����ɫ 

//����LCD��Ҫ����
//Ĭ��Ϊ����
_lcd_dev lcddev;
	
		   
//д�Ĵ�������
//regval:�Ĵ���ֵ
void LCD_WR_REG(u16 regval)
{ 
	LCD->LCD_REG=regval;//д��Ҫд�ļĴ������	 
}
//дLCD����
//data:Ҫд���ֵ
void LCD_WR_DATA(u16 data)
{										    	   
	LCD->LCD_RAM=data;		 
}
//��LCD����
//����ֵ:������ֵ
u16 LCD_RD_DATA(void)
{										    	   
	return LCD->LCD_RAM;		 
}					   
//д�Ĵ���
//LCD_Reg:�Ĵ�����ַ
//LCD_RegValue:Ҫд�������
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{	
	LCD->LCD_REG = LCD_Reg;		//д��Ҫд�ļĴ������	 
	LCD->LCD_RAM = LCD_RegValue;//д������	    		 
}	   
//���Ĵ���
//LCD_Reg:�Ĵ�����ַ
//����ֵ:����������
u16 LCD_ReadReg(u8 LCD_Reg)
{										   
	LCD_WR_REG(LCD_Reg);		//д��Ҫ���ļĴ������
	delay_us(5);		  
	return LCD_RD_DATA();		//���ض�����ֵ
}   
//��ʼдGRAM
void LCD_WriteRAM_Prepare(void)
{
 	LCD->LCD_REG=lcddev.wramcmd;	  
}	 
//LCDдGRAM
//RGB_Code:��ɫֵ
void LCD_WriteRAM(u16 RGB_Code)
{							    
	LCD->LCD_RAM = RGB_Code;//дʮ��λGRAM
}	 
//��ȡ��ĳ�����ɫֵ	 
//x,y:����
//����ֵ:�˵����ɫ
u16 LCD_ReadPoint(u16 x,u16 y)
{
 	u16 r=0;
	if(x>=lcddev.width||y>=lcddev.height)return 0;	//�����˷�Χ,ֱ�ӷ���		   
	LCD_SetCursor(x,y);	    
	
	LCD_WR_REG(0X2E);//9341/6804/3510/1963 ���Ͷ�GRAMָ��	 
 	r=LCD_RD_DATA();								//dummy Read	   
	return r;					//1963ֱ�Ӷ��Ϳ��� 	
}
//LCD��������
//pwm:����ȼ�,0~63.Խ��Խ��.
void LCD_BackLightSet(u8 pwm)
{	
	LCD_WR_REG(0xBE);	//����PWM���
	LCD_WR_DATA(0x05);	//1����PWMƵ��
	LCD_WR_DATA(pwm*2.55);//2����PWMռ�ձ�
	LCD_WR_DATA(0x01);	//3����C
	LCD_WR_DATA(0xFF);	//4����D
	LCD_WR_DATA(0x00);	//5����E
	LCD_WR_DATA(0x00);	//6����F	   	 	 
}
//ɨ�跽����,X���յ�����.
void LCD_EndXSet(u16 x)
{													    		
	LCD_WriteReg(LCD_END_X,x);				//����X������  	 	 
}			 
//LCD������ʾ
void LCD_DisplayOn(void)
{	
	LCD_WR_REG(0X29);	//������ʾ	   	 
}	 
//LCD�ر���ʾ
void LCD_DisplayOff(void)
{	   
	LCD_WR_REG(0X28);	//�ر���ʾ				//�ر�TFT,�൱�ڰѱ���ص����ޱ��⣬����ʾ  	 
} 
//���õ�ǰ��ʾ��
//layer:��ǰ��ʾ�� 
void LCD_SetDisplayLayer(u16 layer)
{	 
	lcddev.sysreg&=~0X0E00;				   	//���֮ǰ������
	lcddev.sysreg|=(layer&0X07)<<9;			//�����µ�ֵ
	LCD_WriteReg(LCD_PREF,lcddev.sysreg);	//дLCD_PREF�Ĵ���	   	 	 
} 
//���õ�ǰ������
//layer:��ǰ��ʾ�� 
void LCD_SetOperateLayer(u16 layer)
{	 
	lcddev.sysreg&=~0X7000;					//���֮ǰ������
	lcddev.sysreg|=(layer&0X07)<<12;		//�����µ�ֵ
	LCD_WriteReg(LCD_PREF,lcddev.sysreg);	//дLCD_PREF�Ĵ���	   	 	 
} 	     
//���ù��λ��
//Xpos:������
//Ypos:������
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	 
	if(lcddev.dir==0)//x������Ҫ�任
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
//����LCD���Զ�ɨ�跽��
//ע��:���ǵ�������,ֻ֧������ɨ������,��֧������ɨ������   	   
void LCD_Scan_Dir(u8 dir)
{			   
  u16 regval=0;
	u16 dirreg=0;
	
	if((lcddev.dir==1&&lcddev.id!=0X1963)||(lcddev.dir==0&&lcddev.id==0X1963))//����ʱ����6804��1963���ı�ɨ�跽������ʱ1963�ı䷽��
	{			   
		switch(dir)//����ת��
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
		case L2R_U2D://������,���ϵ���
			regval|=(0<<7)|(0<<6)|(0<<5); 
			break;
		case L2R_D2U://������,���µ���
			regval|=(1<<7)|(0<<6)|(0<<5); 
			break;
		case R2L_U2D://���ҵ���,���ϵ���
			regval|=(0<<7)|(1<<6)|(0<<5); 
			break;
		case R2L_D2U://���ҵ���,���µ���
			regval|=(1<<7)|(1<<6)|(0<<5); 
			break;	 
		case U2D_L2R://���ϵ���,������
			regval|=(0<<7)|(0<<6)|(1<<5); 
			break;
		case U2D_R2L://���ϵ���,���ҵ���
			regval|=(0<<7)|(1<<6)|(1<<5); 
			break;
		case D2U_L2R://���µ���,������
			regval|=(1<<7)|(0<<6)|(1<<5); 
			break;
		case D2U_R2L://���µ���,���ҵ���
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
//����
//x,y:����
//POINT_COLOR:�˵����ɫ
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);		//���ù��λ�� 
	LCD_WriteRAM_Prepare();	//��ʼд��GRAM
	LCD->LCD_RAM=POINT_COLOR; 
}
//����
//x,y:����
//POINT_COLOR:�˵����ɫ
void GUI_Point(u16 x,u16 y,u16 color)
{
	LCD_SetCursor(x,y);		//���ù��λ�� 
	LCD_WriteRAM_Prepare();	//��ʼд��GRAM
	LCD->LCD_RAM=color; 
}
//���ٻ���
//x,y:����
//color:��ɫ
void LCD_Fast_DrawPoint(u16 x,u16 y,u16 color)
{	  		 		   
	LCD->LCD_REG=lcddev.setycmd;
	LCD->LCD_RAM=y; 
 	LCD->LCD_REG=lcddev.setxcmd; 
	LCD->LCD_RAM=x; 
	LCD->LCD_REG=lcddev.wramcmd; 
	LCD->LCD_RAM=color; 
}	
//����LCD��ʾ����7����,���ܼ򵥵��޸�Ϊ������ʾ��
//dir:0,������1,����
void LCD_Display_Dir(u8 dir)
{
	if(dir==0)			//����
	{
	lcddev.dir=0;	//����
	lcddev.wramcmd=0X2C;	//����д��GRAM��ָ�� 
	lcddev.setxcmd=0X2B;	//����дX����ָ��
	lcddev.setycmd=0X2A;	//����дY����ָ��
	lcddev.width=480;		//���ÿ��480
	lcddev.height=800;		//���ø߶�800  
		
	}
	else 				//����
	{  				
	lcddev.dir=1;	//����	
	lcddev.wramcmd=0X2C;	//����д��GRAM��ָ�� 
	lcddev.setxcmd=0X2A;	//����дX����ָ��
	lcddev.setycmd=0X2B;	//����дY����ָ��
	lcddev.width=800;		//���ÿ��800
	lcddev.height=480;		//���ø߶�480  	
		
	} 
	LCD_Scan_Dir(DFT_SCAN_DIR);	//Ĭ��ɨ�跽��
}
//���ô���,���Զ����û������굽�������Ͻ�(sx,sy).
//sx,sy:������ʼ����(���Ͻ�)
//width,height:���ڿ�Ⱥ͸߶�,�������0!!
//�����С:width*height.
//68042,����ʱ��֧�ִ�������!! 
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
	else if(lcddev.id==0X1963)//1963�������⴦��
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
//��ʼ��lcd
//�ó�ʼ���������Գ�ʼ������ILI93XXҺ��,�������������ǻ���ILI9320��!!!
//�������ͺŵ�����оƬ��û�в���! 
void LCD_Init(void)
{ 
	delay_ms(10);	
	LCD_WR_REG(0XA1);
	lcddev.id=LCD_RD_DATA();
	lcddev.id=LCD_RD_DATA();	//����0X57
	lcddev.id<<=8;	 
	lcddev.id|=LCD_RD_DATA();	//����0X61	
	if(lcddev.id==0X5761)lcddev.id=0X1963;//SSD1963���ص�ID��5761H,Ϊ��������,����ǿ������Ϊ1963
	else lcddev.id=0X0570;//���ô�����V1.8��v2.3�汾��v2.3Ϊ1963����ic��v1.8ΪEPM570T cpld;
	if(lcddev.id==0X1963)	//1963��ʼ��
	{
	LCD_WR_REG(0xE2);		//Set PLL with OSC = 10MHz (hardware),	Multiplier N = 35, 250MHz < VCO < 800MHz = OSC*(N+1), VCO = 300MHz
	LCD_WR_DATA(0x1D);		//����1 
	LCD_WR_DATA(0x02);		//����2 Divider M = 2, PLL = 300/(M+1) = 100MHz
	LCD_WR_DATA(0x04);		//����3 Validate M and N values   
	delay_us(10);
	LCD_WR_REG(0xE0);		// Start PLL command
	LCD_WR_DATA(0x01);		// enable PLL
	delay_us(1);
	LCD_WR_REG(0xE0);		// Start PLL command again
	LCD_WR_DATA(0x03);		// now, use PLL output as system clock	
	delay_us(10);  
	LCD_WR_REG(0x01);		//��λ
	delay_us(10);
	
	LCD_WR_REG(0xE6);		//��������Ƶ��,33Mhz
	LCD_WR_DATA(0x2F);
	LCD_WR_DATA(0xFF);
	LCD_WR_DATA(0xFF);
	
	LCD_WR_REG(0xB0);		//����LCDģʽ
	LCD_WR_DATA(0x20);		//24λģʽ
	LCD_WR_DATA(0x00);		//TFT ģʽ 

	LCD_WR_DATA((SSD_HOR_RESOLUTION-1)>>8);//����LCDˮƽ����
	LCD_WR_DATA(SSD_HOR_RESOLUTION-1);		 
	LCD_WR_DATA((SSD_VER_RESOLUTION-1)>>8);//����LCD��ֱ����
	LCD_WR_DATA(SSD_VER_RESOLUTION-1);		 
	LCD_WR_DATA(0x00);		//RGB���� 
	
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
	
	LCD_WR_REG(0xF0);	//����SSD1963��CPU�ӿ�Ϊ16bit  
	LCD_WR_DATA(0x03);	//16-bit(565 format) data for 16bpp 

	LCD_WR_REG(0x29);	//������ʾ
	//����PWM���  ����ͨ��ռ�ձȿɵ� 
	LCD_WR_REG(0xD0);	//�����Զ���ƽ��DBC
	LCD_WR_DATA(0x00);	//disable

	LCD_WR_REG(0xBE);	//����PWM���
	LCD_WR_DATA(0x05);	//1����PWMƵ��
	LCD_WR_DATA(0xFE);	//2����PWMռ�ձ�
	LCD_WR_DATA(0x01);	//3����C
	LCD_WR_DATA(0x00);	//4����D
	LCD_WR_DATA(0x00);	//5����E 
	LCD_WR_DATA(0x00);	//6����F 
	
	LCD_WR_REG(0xB8);	//����GPIO����
	LCD_WR_DATA(0x03);	//2��IO�����ó����
	LCD_WR_DATA(0x01);	//GPIOʹ��������IO���� 
	LCD_WR_REG(0xBA);
	LCD_WR_DATA(0X01);	//GPIO[1:0]=01,����LCD����
	
	LCD_BackLightSet(100);//��������Ϊ����

	LCD_Display_Dir(1);		//Ĭ��Ϊ����  0:���� 1:����
	
	LCD_Clear(CYAN);
} 
}
//��������
//color:Ҫ���������ɫ
void LCD_Clear(u16 color)
{
	u32 index=0;      
	u32 totalpoint = 384000;//800*480
	LCD_SetCursor(0x00,0x0000);	//���ù��λ��
	LCD_WriteRAM_Prepare();     //��ʼд��GRAM 	  
	for(index=0;index<totalpoint;index++)
	{
		LCD->LCD_RAM=color;	   
	}
}  
//��ָ����������䵥����ɫ
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{          
	u16 i,j;
	u16 xlen=0;
	xlen=ex-sx+1;	   
	for(i=sy;i<=ey;i++)
	{
	 	LCD_SetCursor(sx,i);      				//���ù��λ�� 
		LCD_WriteRAM_Prepare();     			//��ʼд��GRAM	  
		for(j=0;j<xlen;j++)LCD->LCD_RAM=color;	//����LCD	    
	}
}  
//��ָ�����������ָ����ɫ��			 
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color)
{  
	u16 height,width;
	u16 i,j;
	width=ex-sx+1; 		//�õ����Ŀ��
	height=ey-sy+1;		//�߶�
 	for(i=0;i<height;i++)
	{
 		LCD_SetCursor(sx,sy+i);   	//���ù��λ�� 
		LCD_WriteRAM_Prepare();     //��ʼд��GRAM
		for(j=0;j<width;j++)LCD->LCD_RAM=color[i*height+j];//д������ 
	}	  
}  
//����
//x1,y1:�������
//x2,y2:�յ�����  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		LCD_DrawPoint(uRow,uCol);//���� 
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
//���������������ָ����ɫ 
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
//������	  
//(x1,y1),(x2,y2):���εĶԽ�����
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}
//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
void Draw_Circle(u16 x0,u16 y0,u8 r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //�ж��¸���λ�õı�־
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
		//ʹ��Bresenham�㷨��Բ     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
} 

//��һ������
//(x1,y1),(x2,y2):��������ʼ����
//size�������Ĵ�ϸ�̶�
//color����������ɫ
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		gui_fill_circle(uRow,uCol,size,color);//���� 
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

//��ʵ��Բ
//x0,y0:����
//r:�뾶
//color:��ɫ
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

//��ˮƽ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}

//����ֱ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void gui_draw_vline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0,y0+len-1,color);	
}
									  
//��ָ��λ����ʾһ���ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//size:�����С 12/16
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{  							  
    u8 temp,t1,t,size1;
	u16 y0=y;
	u16 colortemp=POINT_COLOR;      			     
	//���ô���		   
	num=num-' ';//�õ�ƫ�ƺ��ֵ
	if(size == 12||size == 16) size1 = size;
	else 
		if(size == 24) size1 = 36;
		else size1 = size*2;
	if(!mode) //�ǵ��ӷ�ʽ
	{
	    for(t=0; t<size1; t++)
	    {   
			if(size==16) temp=asc2_1608[num][t];		 //����1608���� 	                          
	    else if(size == 24) temp=asc2_2412[num][t];
		  else temp=asc2_3216[num][t];		 //����3216���� 	                          
	        for(t1=0; t1<8; t1++)
			{			    
		        if(temp&0x80)POINT_COLOR=colortemp;
				else POINT_COLOR=BACK_COLOR;
				LCD_DrawPoint(x,y);	
				temp<<=1;
				y++;
				if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//��������
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//��������
					break;
				}
			}  	 
	    }    
	}
	else//���ӷ�ʽ
	{
	    for(t=0; t<size1; t++)
	    {   
			if(size==16)temp=asc2_1608[num][t];		 //����1608����
				else if(size==24) temp=asc2_2412[num][t];
					else temp=asc2_3216[num][t];       
	        for(t1=0; t1<8; t1++)
			{			    
		        if(temp&0x80)LCD_DrawPoint(x,y); 
				temp<<=1;
				y++;
				if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//��������
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//��������
					break;
				}
			}  	 
	    }     
	}
	POINT_COLOR=colortemp;	    	   	 	  
}   
//m^n����
//����ֵ:m^n�η�.
u32 LCD_Pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}			 
//��ʾ����,��λΪ0,����ʾ
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//color:��ɫ 
//num:��ֵ(0~4294967295);	 
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
//��ʾ����,��λΪ0,������ʾ
//x,y:�������
//num:��ֵ(0~999999999);	 
//len:����(��Ҫ��ʾ��λ��)
//size:�����С
//mode:
//[7]:0,�����;1,���0.
//[6:1]:����
//[0]:0,�ǵ�����ʾ;1,������ʾ.
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
//��ʾ�ַ���
//x,y:�������
//width,height:�����С  
//size:�����С
//*p:�ַ�����ʼ��ַ		  
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p)
{         
	u8 x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//�˳�
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }  
}

/********************************************/
//�������ϼ�ͷ	 /\(x,y)
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
	lcd_draw_bline(x, y, x1, y1, size, color);	//����1 (x,y):�������,(x1,y1):�յ�����
	lcd_draw_bline(x, y, x2, y1, size, color);	//����2 (x,y):�������,(x2,y1):�յ�����
	lcd_draw_bline(x1, y1, x3, y1, size, color);//����3 (x1,y1):�������,(x3,y1):�յ�����
	lcd_draw_bline(x4, y1, x2, y1, size, color);//����4 (x4,y1):�������,(x2,y1):�յ�����
	lcd_draw_bline(x3, y1, x3, y2, size, color);//����5 (x3,y1):�������,(x3,y2):�յ�����
	lcd_draw_bline(x4, y1, x4, y2, size, color);//����6 (x4,y1):�������,(x4,y2):�յ�����
	lcd_draw_bline(x3, y2, x4, y2, size, color);//����7 (x3,y2):�������,(x4,y2):�յ�����	
}

/********************************************/
//�������¼�ͷ(x3,y2)__7_ (x4,y2)
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
	lcd_draw_bline(x, y, x1, y1, size, color);	//����1 (x,y):�������,(x1,y1):�յ�����
	lcd_draw_bline(x, y, x2, y1, size, color);	//����2 (x,y):�������,(x2,y1):�յ�����
	lcd_draw_bline(x1, y1, x3, y1, size, color);//����3 (x1,y1):�������,(x3,y1):�յ�����
	lcd_draw_bline(x4, y1, x2, y1, size, color);//����4 (x4,y1):�������,(x2,y1):�յ�����
	lcd_draw_bline(x3, y1, x3, y2, size, color);//����5 (x3,y1):�������,(x3,y2):�յ�����
	lcd_draw_bline(x4, y1, x4, y2, size, color);//����6 (x4,y1):�������,(x4,y2):�յ�����
	lcd_draw_bline(x3, y2, x4, y2, size, color);//����7 (x3,y2):�������,(x4,y2):�յ�����	
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
				    temp=codeGB_16[k].Msk[t];//��?��?12��y?Y                          
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
	else//��t?��??��?
	{
		
	   for(k=0; k<100; k++)
	   {
	   		if((codeGB_16[k].Index[0]==font[0])&&(codeGB_16[k].Index[1]==font[1]))
			{
			    for(t=0;t<32;t++)
			    {   												   
				    temp=codeGB_16[k].Msk[t];//��?��?12��y?Y                          
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
//��ʾһ��ָ����С�ĺ���
//X,Y���ֵ�����
//font ����GBK��
//size �����С mode 0������ʾ 1 ������ʾ
void Show_GB24(u16 x,u16 y,u8 *font,u8 size,u8 mode)
{
	u8 temp,t,t1,k;
	u16 y0=y;
	u16 tempcolor=POINT_COLOR;;
	if(mode==0)//?y3��??��?
	{	
		for(k=0; k<200; k++) 
		{
			if((codeGB_24[k].Index[0]==font[0])&&(codeGB_24[k].Index[1]==font[1]))
		    {		
				for(t=0;t<72;t++)//GB24  72
			    {   												   
				    temp=codeGB_24[k].Msk[t];//��?��?12��y?Y                          
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
	else//��t?��??��?
	{
		
	   for(k=0; k<200; k++)
	   {
	   		if((codeGB_24[k].Index[0]==font[0])&&(codeGB_24[k].Index[1]==font[1]))
			{
			    for(t=0;t<72;t++)
			    {   												   
				    temp=codeGB_24[k].Msk[t];//��?��?12��y?Y                          
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
//��ʾһ��ָ����С�ĺ���
//x,y :���ֵ�����
//font:����GBK��
//size:�����С
//mode:0,������ʾ,1,������ʾ	   
void Show_GB32(u16 x,u16 y,u8 *font,u8 size,u8 mode)
{
	u8 temp,t,t1,k;
	u16 y0=y;
//	u8 dzk[32];
	u16 tempcolor;	  
//	if(size!=12&&size!=16)return;//��֧�ֵ�size
//	Get_HzMat(font,dzk,size);//�õ���Ӧ��С�ĵ�������
	if(mode==0)//������ʾ
	{	
		for(k=0; k<200; k++) 
		{
			if((codeGB_32[k].Index[0]==font[0])&&(codeGB_32[k].Index[1]==font[1]))
		    {
		//		for(t=0;t<size*2;t++)
				for(t=0;t<size*4;t++)//GB32x4
			    {   												   
				    temp=codeGB_32[k].Msk[t];//�õ�12����                          
			        for(t1=0;t1<8;t1++)
					{
						if(temp&0x80)LCD_DrawPoint(x,y);
			 			else 
						{
							tempcolor=POINT_COLOR;
							POINT_COLOR=BACK_COLOR;
							LCD_DrawPoint(x,y);
							POINT_COLOR=tempcolor;//��ԭ
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
	else//������ʾ
	{
	   for(k=0; k<200; k++)
	   {
	   		if((codeGB_32[k].Index[0]==font[0])&&(codeGB_32[k].Index[1]==font[1]))
			{
			    for(t=0;t<size*4;t++)
			    {   												   
				    temp=codeGB_32[k].Msk[t];//�õ�12����                          
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




//��ָ��λ�ÿ�ʼ��ʾһ���ַ���	    
//֧���Զ�����
//(x,y):��ʼ����
//width,height:����
//str  :�ַ���
//size :�����С
//mode:0,�ǵ��ӷ�ʽ;1,���ӷ�ʽ    	   		   
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode)
{					
	u16 x0=x;
	u16 y0=y;							  	  
    u8 bHz=0;     //�ַ���������  	    				    				  	  
    while(*str!=0)//����δ����
    { 
        if(!bHz)
        {
	        if(*str>0x80)bHz=1;//���� 
	        else              //�ַ�
	        {      
                if(x>(x0+width-size/2))//����
				{				   
					y+=size;
					x=x0;	   
				}							    
		        if(y>(y0+height-size))break;//Խ�緵��      
		        if(*str==13)//���з���
		        {         
		            y+=size;
					x=x0;
		            str++; 
		        }  
		        else LCD_ShowChar(x,y,*str,size,mode);//��Ч����д�� 
				str++; 
		        x+=size/2; //�ַ�,Ϊȫ�ֵ�һ�� 
	        }
        }
				else//���� 
        {     
            bHz=0;//�к��ֿ� 
            if(x>(x0+width-size))//����
			{	    
				y+=size;
				x=x0;		  
			}
	    if(y>(y0+height-size))break;//Խ�緵��
			if(size==16)
				Show_GB16(x,y,str,size,mode);	
			else if(size==24)
				Show_GB24(x,y,str,size,mode);
			else if(size==32)	 
				Show_GB32(x,y,str,size,mode);
	        str+=2; 
	        x+=size;//��һ������ƫ��	    
        }						 
    }   
}  			 		 
//��ָ����ȵ��м���ʾ�ַ���
//����ַ����ȳ�����len,����Show_Str��ʾ
//len:ָ��Ҫ��ʾ�Ŀ��			  
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

u16 GU16_key_color = LIGHTGRAY;		//��ť������ɫ

u16 GU16_bordercolor_light = WHITE;	//�����߿�ǳɫ������ɫ
u16 GU16_bordercolor_dark = LGRAY;	//�����߿���ɫ������ɫ

/********************************************************************/
//�������ܣ����ð���
//(x,y)���ΰ�����������
//len:��������
//wid:�������
/********************************************************************/
void Put_AnJian(u16 x, u16 y, u16 len, u16 wid)
{
	u16 lx_inside, ly_inside, rx_inside, ry_inside;
	lx_inside = x - len/2;
	ly_inside = y - wid/2;
	rx_inside = x + len/2;
	ry_inside = y + wid/2;
	LCD_Fill(lx_inside,ly_inside,rx_inside,ry_inside,GU16_key_color); 						//��������ɫ

	LCD_Fill(lx_inside-1, ly_inside-1, lx_inside-1, ry_inside+1, GU16_bordercolor_light);	//����ֱ�߿���,��ɫ
	LCD_Fill(lx_inside-2, ly_inside-2, lx_inside-2, ry_inside+2, GU16_bordercolor_light);
	LCD_Fill(lx_inside-3, ly_inside-3, lx_inside-3, ry_inside+3, GU16_bordercolor_light);
	
	LCD_Fill(lx_inside-1, ly_inside-1, rx_inside+1, ly_inside-1, GU16_bordercolor_light);	//���ϱ߿���,ǳɫ
	LCD_Fill(lx_inside-2, ly_inside-2, rx_inside+2, ly_inside-2, GU16_bordercolor_light);
	LCD_Fill(lx_inside-3, ly_inside-3, rx_inside+3, ly_inside-3, GU16_bordercolor_light);	

	LCD_Fill(rx_inside+1, ly_inside,   rx_inside+1, ry_inside+1, GU16_bordercolor_dark);		//���Ҵ�ֱ�߿���,��ɫ
	LCD_Fill(rx_inside+2, ly_inside-1, rx_inside+2, ry_inside+2, GU16_bordercolor_dark);
	LCD_Fill(rx_inside+3, ly_inside-2, rx_inside+3, ry_inside+3, GU16_bordercolor_dark);
	
	LCD_Fill(lx_inside-1, ry_inside,   rx_inside+1, ry_inside-1, GU16_bordercolor_dark);		//���±߿���,��ɫ
	LCD_Fill(lx_inside-2, ry_inside+1, rx_inside+2, ry_inside+2, GU16_bordercolor_dark);
	LCD_Fill(lx_inside-3, ry_inside+2, rx_inside+3, ry_inside+3, GU16_bordercolor_dark);	
	if (len > 50) //���������ȴ���50ʱ���߿��߱��
	{
		LCD_Fill(lx_inside-4, ly_inside-4, lx_inside-4, ry_inside+4, GU16_bordercolor_light); //����ֱ�߿���,ǳɫ
		LCD_Fill(lx_inside-5, ly_inside-5, lx_inside-5, ry_inside+5, GU16_bordercolor_light);
		LCD_Fill(lx_inside-6, ly_inside-6, lx_inside-6, ry_inside+6, GU16_bordercolor_light);
	
		LCD_Fill(lx_inside-4, ly_inside-4, rx_inside+4, ly_inside-4, GU16_bordercolor_light); //���ϱ߿���,ǳɫ
		LCD_Fill(lx_inside-5, ly_inside-5, rx_inside+5, ly_inside-5, GU16_bordercolor_light);
		LCD_Fill(lx_inside-6, ly_inside-6, rx_inside+6, ly_inside-6, GU16_bordercolor_light);
	
		LCD_Fill(rx_inside+4, ly_inside-3, rx_inside+4, ry_inside+4, GU16_bordercolor_dark);	//���Ҵ�ֱ�߿���,��ɫ
		LCD_Fill(rx_inside+5, ly_inside-4, rx_inside+5, ry_inside+5, GU16_bordercolor_dark);
		LCD_Fill(rx_inside+6, ly_inside-5, rx_inside+6, ry_inside+6, GU16_bordercolor_dark);
	
		LCD_Fill(lx_inside-4, ry_inside+3, rx_inside+4, ry_inside+4, GU16_bordercolor_dark);	 //���±߿���,��ɫ
		LCD_Fill(lx_inside-5, ry_inside+4, rx_inside+5, ry_inside+5, GU16_bordercolor_dark);
		LCD_Fill(lx_inside-6, ry_inside+5, rx_inside+6, ry_inside+6, GU16_bordercolor_dark);
	}	
}

/********************************************************************/
//����������ʱ��״̬
//(x,y)���ΰ�����������
//len:��������
//wid:�������
/********************************************************************/
void AnJianPressed(u16 x, u16 y, u16 len, u16 wid)
{
	u16 lx_inside, ly_inside, rx_inside, ry_inside;
	lx_inside = x - len/2;
	ly_inside = y - wid/2;
	rx_inside = x + len/2;
	ry_inside = y + wid/2;
	LCD_Fill(lx_inside,ly_inside,rx_inside,ry_inside,GU16_key_color); 						//��������ɫ

	LCD_Fill(lx_inside-1, ly_inside-1, lx_inside-1, ry_inside+1, GU16_bordercolor_dark);	//����ֱ�߿���
	LCD_Fill(lx_inside-2, ly_inside-2, lx_inside-2, ry_inside+2, GU16_bordercolor_dark);
	LCD_Fill(lx_inside-3, ly_inside-3, lx_inside-3, ry_inside+3, GU16_bordercolor_dark);

	LCD_Fill(lx_inside-1, ly_inside-1, rx_inside+1, ly_inside-1, GU16_bordercolor_dark);	//���ϱ߿���
	LCD_Fill(lx_inside-2, ly_inside-2, rx_inside+2, ly_inside-2, GU16_bordercolor_dark);
	LCD_Fill(lx_inside-3, ly_inside-3, rx_inside+3, ly_inside-3, GU16_bordercolor_dark);
	
	LCD_Fill(rx_inside+1, ly_inside,   rx_inside+1, ry_inside+1, GU16_bordercolor_light);	//���Ҵ�ֱ�߿���
	LCD_Fill(rx_inside+2, ly_inside-1, rx_inside+2, ry_inside+2, GU16_bordercolor_light);
	LCD_Fill(rx_inside+3, ly_inside-2, rx_inside+3, ry_inside+3, GU16_bordercolor_light);
	
	LCD_Fill(lx_inside-1, ry_inside,   rx_inside+1, ry_inside-1, GU16_bordercolor_light);	//���±߿���
	LCD_Fill(lx_inside-2, ry_inside+1, rx_inside+2, ry_inside+2, GU16_bordercolor_light);
	LCD_Fill(lx_inside-3, ry_inside+2, rx_inside+3, ry_inside+3, GU16_bordercolor_light);
	if (len > 50)  //���������ȴ���50ʱ���߿��߱��
	{
		LCD_Fill(lx_inside-4, ly_inside-4, lx_inside-4, ry_inside+4, GU16_bordercolor_dark);	//����ֱ�߿���
		LCD_Fill(lx_inside-5, ly_inside-5, lx_inside-5, ry_inside+5, GU16_bordercolor_dark);
		LCD_Fill(lx_inside-6, ly_inside-6, lx_inside-6, ry_inside+6, GU16_bordercolor_dark);

		LCD_Fill(lx_inside-4, ly_inside-4, rx_inside+4, ly_inside-4, GU16_bordercolor_dark);	//���ϱ߿���
		LCD_Fill(lx_inside-5, ly_inside-5, rx_inside+5, ly_inside-5, GU16_bordercolor_dark);
		LCD_Fill(lx_inside-6, ly_inside-6, rx_inside+6, ly_inside-6, GU16_bordercolor_dark);

		LCD_Fill(rx_inside+4, ly_inside-3, rx_inside+4, ry_inside+4, GU16_bordercolor_light); //���Ҵ�ֱ�߿���
		LCD_Fill(rx_inside+5, ly_inside-4, rx_inside+5, ry_inside+5, GU16_bordercolor_light);
		LCD_Fill(rx_inside+6, ly_inside-5, rx_inside+6, ry_inside+6, GU16_bordercolor_light);

		LCD_Fill(lx_inside-4, ry_inside+3, rx_inside+4, ry_inside+4, GU16_bordercolor_light); //���±߿���
		LCD_Fill(lx_inside-5, ry_inside+4, rx_inside+5, ry_inside+5, GU16_bordercolor_light);
		LCD_Fill(lx_inside-6, ry_inside+5, rx_inside+6, ry_inside+6, GU16_bordercolor_light); 	
	}		
}
/* ����ʮ����(0-7)==>ʮ������λת����������ʾ�������������ң�����ʮ������λ˳���ǵ��� */
u8 const  DCB2HEX_TAB[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
u16  disp_color;
u16	 back_color;

u8  GUI_LoadLine(u32 x, u32 y, u8 *dat, u32 no)
{  u8   bit_dat;  
   u16   i;
   u16  bakc;

   /* �������� */
   if(x>=800) return(0);
   if(y>=480) return(0);
   
   for(i=0; i<no; i++)
   {  /* �ж��Ƿ�Ҫ��ȡ�������� */
      if( (i%8)==0 ) bit_dat = *dat++;
     
      /* ������Ӧ�ĵ�Ϊcolor��Ϊback_color */
      if( (bit_dat&DCB2HEX_TAB[i&0x07])==0 ) GUI_CopyColor(&bakc, back_color); 
         else  GUI_CopyColor(&bakc, disp_color);
      GUI_Point(x, y, bakc);       
     
      if( (++x)>=800 ) return(0);
   }
   
   return(1);
}

/****************************************************************************
* ���ƣ�GUI_LoadPic()
* ���ܣ������ɫͼ�����ݡ�
* ��ڲ����� x		ָ����ʾλ�ã�x����(x������8��������)
*           y		ָ����ʾλ�ã�y����
*           dat		Ҫ�����ʾ������
*           hno     Ҫ��ʾ���еĵ����
*           lno     Ҫ��ʾ���еĵ����
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  GUI_LoadPic(u32 x, u32 y, u8 *dat, u32 hno, u32 lno)
{  u32  i; 
   for(i=0; i<lno; i++)
   {  GUI_LoadLine(x, y, dat, hno);				// ���һ������
      y++;										// ��ʾ��һ��
      dat += (hno>>3);							// ������һ�е�����
      if( (hno&0x07)!=0 ) dat++;
   }
}

void  GUI_SetColor(u16 color1, u16 color2)
{  GUI_CopyColor(&disp_color, color1);
   GUI_CopyColor(&back_color, color2);  
}




