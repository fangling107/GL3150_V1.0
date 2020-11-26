#include "touch.h" 
#include "24cxx.h"
#include "ra8870.h"
#include "stdlib.h"
#include "math.h"

//==============================================
u16 X1,Y1,X2,Y2,X3,Y3,X4,Y4;//У׼�����
u16 LCD_X,LCD_Y;		//ʵ������ֵ
u16 ADC_value_X,ADC_value_Y;//��������ֵ
float xfac;					
float yfac;
short xoff;
short yoff;
u8 sta;//����״̬//b7:����1/�ɿ�0; 
	                     //b6:0,û�а�������;1,�а�������.
u8 touch_flag;	
u8 keystatus;
//����У׼����										    
void TP_Save_Adjdata(void)
{
	s32 temp;			 
	//����У�����!		   							  
	temp=xfac*100000000;//����xУ������      
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE,temp,4);   
	temp=yfac*100000000;//����yУ������    
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+4,temp,4);
	//����xƫ����
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+8,xoff,2);		    
	//����yƫ����
	AT24CXX_WriteLenByte(SAVE_ADDR_BASE+10,yoff,2);	
	//���津������
	//AT24CXX_WriteOneByte(SAVE_ADDR_BASE+12,touchtype);	
	temp=0X0A;//���У׼����
	AT24CXX_WriteOneByte(SAVE_ADDR_BASE+13,temp); 
}

//�õ�������EEPROM�����У׼ֵ
//����ֵ��1���ɹ���ȡ����
//        0����ȡʧ�ܣ�Ҫ����У׼
u8 TP_Get_Adjdata(void)
{					  
	s32 tempfac;
	tempfac=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+13);//��ȡ�����,���Ƿ�У׼���� 		 
	if(tempfac==0X0A)//�������Ѿ�У׼����			   
	{    												 
		tempfac=AT24CXX_ReadLenByte(SAVE_ADDR_BASE,4);		   
		xfac=(float)tempfac/100000000;//�õ�xУ׼����
		tempfac=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+4,4);			          
		yfac=(float)tempfac/100000000;//�õ�yУ׼����
	    //�õ�xƫ����
		xoff=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+8,2);			   	  
 	    //�õ�yƫ����
		yoff=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+10,2);				 	  
 		//tp_dev.touchtype=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+12);//��ȡ�������ͱ��				 
		return 1;	 
	}
	return 0;
}	

//bit touch_valid = 0;
//Subroutine:	Detect_TP_X_Y coordinate 
//==============================================================================
u8 ADC_X(void)
{	u8 temp;
	LCD_REG=(0x72);//TPXH	 X_coordinate high byte
	//Chk_Busy();
	temp=LCD_RAM;
	return temp;
}

u8 ADC_Y(void)
{	u8 temp;
	LCD_REG=(0x73);//TPYH	  Y_coordinate high byte
	//Chk_Busy();
	temp=LCD_RAM;
	return temp;
}

u8 ADC_XY(void)
{	u8 temp;
 	LCD_REG=(0x74);//TPXYL	  bit[3:2] Y_coordinate low byte  bit[1:0] X_coordinate low byte 
	temp=LCD_RAM;
	return temp;
} 
//��ȡX��ADֵ
u16 Get_X_ADC_Value_10bit(void)
{
   u16 temp,temp1;
   
   temp=0;
   temp|=ADC_X();
   temp1=(temp<<2);
   temp= temp1|((ADC_XY()&0x03));
   
   return temp;   
}

//��ȡY��ADֵ
u16 Get_Y_ADC_Value_10bit(void)
{
   u16 temp,temp1;
   
   temp=0;
   temp|=ADC_Y();
   temp1=(temp<<2);
   temp=temp1|((ADC_XY()>>2)&0x03);
   
   return temp;   
}
//REG[80h]~REG[83h] ͼ�ι��λ��
void Graphic_Cursor_Coordinate(u16 X,u16 Y)
{
    LCD_REG=(0x80);
	LCD_RAM=(X);  
    LCD_REG=(0x81);	   
	LCD_RAM=(X>>8);
  
    LCD_REG=(0x82);
	LCD_RAM=(Y);  
    LCD_REG=(0x83);	   
	LCD_RAM=(Y>>8);
}
//ȡX2 X3 X4���м�ֵ ���Ƹ�X1 �ٽ�ADC_value_X = X1; ADC_value_Y = Y1;
void MidValueCalculate(void)
{
	if (X2>X3)
	{
		if (X3>X4)
		{X1=X3;}	
		else
		{
		   if (X2>X4)
			{ X1=X4;}
			else	
			{ X1=X2;}
		}		
	}			
 else
	{
		if (X3<X4)
		{ X1=X3;}	
		else
		{
			if (X2<X4)
			{ X1=X4;}
			else	
			{ X1=X2; }
		}	
	}
			
 if (Y2>Y3)
	{
		if (Y3>Y4)
		{Y1=Y3;}	
		else
		{
		   if (Y2>Y4)
			{ Y1=Y4;}
			else	
			{ Y1=Y2;}
		}		
	}			
 else
	{
		if (Y3<Y4)
		{ Y1=Y3;}	
		else
		{
			if (Y2<Y4)
			{ Y1=Y4;}
			else	
			{ Y1=Y2; }
		}									
   }

 	  ADC_value_X = X1;
    ADC_value_Y = Y1;

 }
 

// TP controller application function

void Access_TP_coordinate(u8 tp)	//tp=1 ת��ʵ������ֵ tp=0������ת��
{

 clear_TP_interrupt();
 Delay1ms(100);  //delay after clear_TP_interrupt		 
 if(Touch_Status())
  {	 			
   X2=Get_X_ADC_Value_10bit();  
   Y2=Get_Y_ADC_Value_10bit();   
	 clear_TP_interrupt(); 
   Delay1ms(10);  //delay after clear_TP_interrupt
   if(Touch_Status())
      { 				
       X3=Get_X_ADC_Value_10bit();
       Y3=Get_Y_ADC_Value_10bit();    
			 clear_TP_interrupt();
			 Delay1ms(10);  //delay after clear_TP_interrupt

       if(Touch_Status())//�а�������
         {					
          X4 =Get_X_ADC_Value_10bit();
          Y4 =Get_Y_ADC_Value_10bit();										 
          MidValueCalculate();    //ȡ�м�ֵ������ֵ�� ADC_value_X ADC_value_Y   
          if(tp) Get_XY_Calibration(); 	//�õ�ʵ������ֵ	
					touch_flag=1;	
         }	
				else 
				 {					
					ADC_value_X=0; ADC_value_Y=0;
					LCD_X=0;LCD_Y=0;					
				 }
     }
     else
     {
   	  ADC_value_X=0; ADC_value_Y=0;
   	  LCD_X=0;LCD_Y=0;	
						 
   	 }
		
	 }	   
	else
	 {
		ADC_value_X=0; ADC_value_Y=0;
		LCD_X=0;LCD_Y=0;
		
	 }
	
}

	//��������ɨ��
//tp:0,��Ļ����;1,��������(У׼�����ⳡ����)
//����ֵ:��ǰ����״̬.
//0,�����޴���;1,�����д���
u8 TP_Scan(void)
{			   
	if(Touch_Status())//�а�������
	{
		Access_TP_coordinate(0);//���а���ɨ��,��ȡ��������
		if((sta&TP_PRES_DOWN)==0)//֮ǰû�б�����
		{		 
			sta=TP_PRES_DOWN|TP_CATH_PRES;//��������  				   			 
		}			   
	}else
	{
		if(sta&TP_PRES_DOWN)//֮ǰ�Ǳ����µ�
		{
			sta&=~(1<<7);//��ǰ����ɿ�	
		}
		else//֮ǰ��û�б�����
		{			
			sta&=0XE0;	//�������Ч���		
		}	    
	}
	return sta&TP_PRES_DOWN;//���ص�ǰ�Ĵ���״̬
}	


//��ʾXY������ADֵ
void Trace_XY_Vaule(void)
{	  
      u8 temp;
	  u16 temp1;

	  temp1=ADC_value_X;
	  temp=(ADC_value_X>>8);	  		
	  XY_Coordinate(0,60);  //����������ʾλ��
	  Show_String("X=0x");  
	  Printf_Hex(temp);		
	  temp=temp1;
	  Printf_Hex(temp);	
    Chk_Busy();

	  temp1=ADC_value_Y;
	  temp=(ADC_value_Y>>8);
		XY_Coordinate(0,100);  //����������ʾλ��
	  Show_String("Y=0x");
	  Printf_Hex(temp);		
	  temp=temp1;
	  Printf_Hex(temp);
	  Chk_Busy();
}


//================================================
//����ֵת��Ϊʵ������ֵ
//================================================

void Get_XY_Calibration(void)
{
		s16 temp1=0;
		temp1=xfac*ADC_value_X;
		LCD_X = temp1+xoff;
		
		temp1=yfac*ADC_value_Y;
		LCD_Y = temp1+yoff;
}

//==============================================================================
//Subroutine:	Show Hex	
//==============================================================================
void Printf_Hex(u8 buf)
{
	u8 temp;
	temp=buf;
	temp = (temp >>4) & 0x0F;
	if(temp < 0x0A)
	{temp |= 0x30;
	}
	else temp = temp + 0x37;
	LCD_RAM=(temp);
  Chk_Busy();

	temp=buf;
	temp = temp & 0x0F;
	if(temp < 0x0A)
	{temp |= 0x30;
	}
	else temp = temp + 0x37;
	LCD_RAM=(temp);
  Chk_Busy();
}
 //�����ʾ����ֵ
void Trace_coordinate(void)
{
			  		
	  XY_Coordinate(0,140);  //����������ʾλ��
	  Show_String("LCD_X=");		//��ʾ�ַ�
		Print_Decimal(LCD_X);
		
	  XY_Coordinate(0,180);  //����������ʾλ��	  
	  Show_String("LCD_Y=");
	  Print_Decimal(LCD_Y); 
		Graphic_Mode();
}	

//16����ת��ʮ�������
void Print_Decimal(u16 buffer)
{
	u16 temp,temp1;
     
    temp=buffer/100;
    temp1= buffer%100;

    temp|=0x30;
		LCD_RAM=(temp);
		Chk_Busy();
  
    temp=temp1/10;	
		temp|=0x30;
    LCD_RAM=(temp);
		Chk_Busy();

    temp=temp1%10;	
		temp|=0x30;
    LCD_RAM=(temp);
		Chk_Busy();

}

//У׼����
/*
void XY_Calibration_Function(void)
{
   u8 state =0;
   Active_Window(0,479,0,271);
   Text_Background_Color(color_white);  
   Memory_Clear();
   Chk_Busy();

   Text_Foreground_Color(color_blue);
   Circle_Coordinate_Radius(Test_Dot1_X,Test_Dot1_Y,1);	//��һ��ʵ��Բ��
   Draw_circle_fill();
   Chk_Busy();

   Text_Foreground_Color(color_red);// 
   Circle_Coordinate_Radius(Test_Dot1_X,Test_Dot1_Y,4);//�������߿�	
   Draw_circle();
   Chk_Busy();
   Text_Foreground_Color(color_red);// 
   Circle_Coordinate_Radius(Test_Dot1_X,Test_Dot1_Y,5);//�ٻ�һ�������߿�	
   Draw_circle();
   Chk_Busy();


   // �Ե�һ�������У׼  
	 state=1;
		do{ 
				Access_TP_coordinate(); 				//ȡ�м�ֵ������ֵ�� ADC_value_X ADC_value_Y  
				if((X1>(Xa-20))&&(X1<(Xa+20))) 
				{
					if((Y1>(Ya-20))&&(Y1<(Ya+20)))
					{
						XVa=X1; YVa=Y1; state=0;	//�ж���Χ���˳�ѭ��
					}
				}			 
			 //Trace_XY_Vaule();							//��ʾXY������ADֵ
			// Trace_coordinate();
			 Delay1ms(200);		
      }while(state); 

   //========================================================//    
 
   Active_Window(0,479,0,271);   
   Text_Background_Color(color_white);  
   Memory_Clear();
   Chk_Busy();
 

   Text_Foreground_Color(color_blue);
   Circle_Coordinate_Radius(Test_Dot2_X,Test_Dot2_Y,1);	
   Draw_circle_fill();
   Chk_Busy();

   Text_Foreground_Color(color_red);// 
   Circle_Coordinate_Radius(Test_Dot2_X,Test_Dot2_Y,4);	
   Draw_circle();
   Chk_Busy();
   Circle_Coordinate_Radius(Test_Dot2_X,Test_Dot2_Y,5);	
   Draw_circle();
   Chk_Busy();

   ///
	 state=1;
		do{ 

					Access_TP_coordinate(); 
          if((X1>(Xb-20))&&(X1<(Xb+20)))  
          {
						if((Y1>(Yb-20))&&(Y1<(Yb+20)))
						{
							XVb=X1; YVb=Y1; state=0;							
						}
          }
           Delay1ms(200);				
          }
         while(state);	
				OFFSET_X = (Xa-XVa)+(Xb-XVb); //�D�XX������
				OFFSET_X /= 2;

				OFFSET_Y = (Ya-YVa)+(Yb-YVb); //�D�XY������
				OFFSET_Y /= 2;	
				TP_Save_Adjdata();					
}*/
//ָ��λ�û�У׼��
void drawpoint(u16 x,u16 y)
{			
	 Text_Foreground_Color(color_blue);
	 Circle_Coordinate_Radius(x,y,1);	//��һ��ʵ��Բ��
	 Draw_circle_fill();
	 Chk_Busy();

	 Text_Foreground_Color(color_red);// 
	 Circle_Coordinate_Radius(x,y,4);//�������߿�	
	 Draw_circle();
	 Chk_Busy();
	 Text_Foreground_Color(color_red);// 
	 Circle_Coordinate_Radius(x,y,5);//�ٻ�һ�������߿�	
	 Draw_circle();
	 Chk_Busy();
}

void XY_Calibration_Function(void)
{
	u16 outtime=0;
	u16 pos_temp[4][2];//���껺��ֵ
	u8  cnt=0;
	u16 d1,d2;
	u32 tem1,tem2;
	double fac;

	//����
	Active_Window(0,479,0,271);
	Text_Background_Color(color_white);  
	Memory_Clear();
	Chk_Busy();	
	drawpoint(20,20);//(20,20)��λ�û���һ��У׼��
	while(1)
	{
		outtime=0;
		TP_Scan();//���д���ɨ��
		if((sta&0xc0)==TP_CATH_PRES)//��Ч����
		{
					
			pos_temp[cnt][0]=X1;
			pos_temp[cnt][1]=Y1;			
			cnt++;		
			sta&=~(1<<6);//��ǰ����Ѿ����������.
			switch(cnt)
			{
				case 1:						 
					Active_Window(0,479,0,271);
					Text_Background_Color(color_white);  
					Memory_Clear();
					Chk_Busy();				//�����1 
					drawpoint(460,20);	//����2
					
					break;
				case 2:
					Active_Window(0,479,0,271);
					Text_Background_Color(color_white);  
					Memory_Clear();
					Chk_Busy();				//�����2 
					drawpoint(20,252);	//����3
					break;
				case 3:
					Active_Window(0,479,0,271);
					Text_Background_Color(color_white);  
					Memory_Clear();
					Chk_Busy();				//�����3 
					drawpoint(460,252);	//����4
					break;
				case 4://ȫ���ĸ��㶼�Ѿ��õ�  //�Ա����
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,2�ľ���
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�3,4�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0)//���ϸ�
					{
						cnt=0;
 				    Active_Window(0,479,0,271);
						Text_Background_Color(color_white);  
						Memory_Clear();
						Chk_Busy();				//�����4 
   	 				drawpoint(20,20);//(20,20)��λ�û���һ��У׼��
 						//��ʾ�ĸ����������Ϣ
 						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,3�ľ���
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�2,4�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//���ϸ�
					{
						cnt=0;
 				    Active_Window(0,479,0,271);
						Text_Background_Color(color_white);  
						Memory_Clear();
						Chk_Busy();				//�����4 
   	 				drawpoint(20,20);//(20,20)��λ�û���һ��У׼��
 						//��ʾ����   
						continue;
					}
					//��ȷ��
					//�Խ������
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//�õ�1,4�ľ���
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//�õ�2,3�ľ���
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//���ϸ�
					{
						cnt=0;
 				    Active_Window(0,479,0,271);
						Text_Background_Color(color_white);  
						Memory_Clear();
						Chk_Busy();				//�����4 
   	 				drawpoint(20,20);//(20,20)��λ�û���һ��У׼��
 						//TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����   
						continue;
					}//��ȷ��
					//������
					xfac=(float)(480-40)/(pos_temp[1][0]-pos_temp[0][0]);//�õ�xfac	X2-X1	 
					xoff=(480-xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//�õ�xoff
						  
					yfac=(float)(272-40)/(pos_temp[2][1]-pos_temp[0][1]);//�õ�yfac
					yoff=(272-yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//�õ�yoff 
					if(abs(xfac)>2||abs(yfac)>2)//������Ԥ����෴��.
					{
						cnt=0;
 				    Active_Window(0,479,0,271);
						Text_Background_Color(color_white);  
						Memory_Clear();
						Chk_Busy();				//�����4 
   	 				drawpoint(20,20);//(20,20)��λ�û���һ��У׼��
						//LCD_ShowString(40,26,lcddev.width,lcddev.height,16,"TP Need readjust!");								    
						continue;
					}
					Active_Window(0,479,0,271);
					Text_Background_Color(color_white);  
					Memory_Clear();				
					Delay1ms(1000);
					Text_Mode();
					Text_Foreground_Color(color_blue);
					Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
					External_CGROM();
					External_CGROM_GB();
					Active_Window(0,480,0,60);//�趨��ʾ����
					XY_Coordinate(100,170);
					Bold_Font();//����Ӵ�
					Show_String("Touch Screen Adjust OK!");
					NoBold_Font();//ȡ������Ӵ�						
					Graphic_Mode();				
					TP_Save_Adjdata();
					Delay1ms(100);
					TP_Get_Adjdata();//��ȡԭʼֵ
					return;//У�����
			}

		}
		Delay1ms(10);
		outtime++;
		if(outtime>1000)//10Sû�а�������
		{
			TP_Get_Adjdata();//��ȡԭʼֵ
			break;//�˳�У׼����
	 	}
	}

}
//����ɨ�� ���ȴ�����
void KeyScan(void) 
{
	Access_TP_coordinate(1);	
	keystatus = Touch_Status(); 				
	while(keystatus)
	{
		clear_TP_interrupt();
		Delay1ms(50);
		keystatus = Touch_Status();		
	}
}

