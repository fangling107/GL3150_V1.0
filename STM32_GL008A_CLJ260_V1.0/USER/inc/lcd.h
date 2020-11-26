#ifndef __LCD_H
#define __LCD_H		
#include "setup.h" 
#include "stdlib.h"
//////////////////////////////////////////////////////////////////////////////////	 
//7�� TFTҺ������	  	
//********************************************************************************
//��
//////////////////////////////////////////////////////////////////////////////////	 

//LCD��Ҫ������
typedef struct  
{										    
	u16 width;			//LCD ���
	u16 height;			//LCD �߶�
	u16 id;				//LCD ID��ATK-7' TFTLCD��֧��id����
	u8  dir;			//���������������ƣ�0��������1��������	
	u16 sysreg;			//PREF�Ĵ��������õ�ǰ����ҳ����ʾҳ�ͱ���ȣ�ATK-7' TFTLCD�ݲ�֧�ֺ��������ã�ͳһΪ��������	
	u8	wramcmd;		//��ʼдgramָ��
	u8  setxcmd;		//����x����ָ��
	u8  setycmd;		//����y����ָ��	 
}_lcd_dev; 	  

//LCD����
extern _lcd_dev lcddev;	//����LCD��Ҫ����
//LCD�Ļ�����ɫ�ͱ���ɫ	   
extern u16  POINT_COLOR;//Ĭ�Ϻ�ɫ    
extern u16  BACK_COLOR; //������ɫ.Ĭ��Ϊ��ɫ


//////////////////////////////////////////////////////////////////////////////////	 
//-----------------LCD�˿ڶ���---------------- 
//#define	LCD_LED PBout(0) 	//LCD���ⲻ��IO�ڿ���,���������Լ�����,ͨ��ָ������   
#define	LCD_RST PBout(0) 		//LCD��λ��,�Ϳ����干��,��������ΪPB0,��ʵ��û���õ�    
//LCD��ַ�ṹ��
typedef struct
{
	u16 LCD_REG;
	u16 LCD_RAM;
} LCD_TypeDef;
//ʹ��NOR/SRAM�� Bank1.sector4,��ַλHADDR[27,26]=11 A10��Ϊ�������������� 
//ע������ʱSTM32�ڲ�������һλ����! 111110=0X3E			    
#define LCD_BASE        ((u32)(0x6C000000 | 0x000007FE))
#define LCD             ((LCD_TypeDef *) LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////
//LCD ָ���
#define LCD_CUR_Y  		0X00
#define LCD_CUR_X  		0X01
#define LCD_PIXELS 		0X02
#define LCD_END_X 		0X03
#define LCD_PREF 		0X05
#define LCD_MIRROR 		0X07


//////////////////////////////////////////////////////////////////////////////////
	 
//ɨ�跽����
#define L2R_U2D  0 //������,���ϵ���
#define L2R_D2U  1 //������,���µ���
#define R2L_U2D  2 //���ҵ���,���ϵ���
#define R2L_D2U  3 //���ҵ���,���µ���

#define U2D_L2R  4 //���ϵ���,������
#define U2D_R2L  5 //���ϵ���,���ҵ���
#define D2U_L2R  6 //���µ���,������
#define D2U_R2L  7 //���µ���,���ҵ���	 

#define DFT_SCAN_DIR  L2R_U2D  //Ĭ�ϵ�ɨ�跽��

//������ɫ
#define WHITE         	 0xFFFF	 //��ɫ
#define BLACK         	 0x0000	 //��ɫ 
#define BLUE         	 0x001F  //��ɫ
#define BRED             0XF81F	 //��ɫ
#define GRED 			 0XFFE0	 //��ɫ
#define GBLUE			 0X07FF	 //ǳ��ɫ
#define RED           	 0xF800	 //��ɫ
#define MAGENTA       	 0xF81F	 //��ɫ
#define GREEN         	 0x07E0	 //��ɫ
#define CYAN          	 0x7FFF	 //����ɫ
#define YELLOW        	 0xFFE0	 //��ɫ
#define BROWN 			 0XBC40  //��ɫ
#define BRRED 			 0XFC07  //�غ�ɫ
#define GRAY  			 0X8430  //��ɫ
//GUI��ɫ

#define DARKBLUE      	 0X01CF	 //����ɫ
#define LIGHTBLUE      	 0X7D7C	 //ǳ��ɫ  
#define GRAYBLUE       	 0X5458  //����ɫ
//������ɫΪPANEL����ɫ 
 
#define LIGHTGREEN     	 0X841F //ǳ��ɫ
#define LIGHTGRAY        0XEF5B //ǳ��ɫ(PANNEL)
#define LGRAY 			 0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ

#define LGRAYBLUE        0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE           0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)

//LCD�ֱ�������
#define SSD_HOR_RESOLUTION		800		//LCDˮƽ�ֱ���
#define SSD_VER_RESOLUTION		480		//LCD��ֱ�ֱ���
//LCD������������
#define SSD_HOR_PULSE_WIDTH		1		//ˮƽ����
#define SSD_HOR_BACK_PORCH		46		//ˮƽǰ��
#define SSD_HOR_FRONT_PORCH		210		//ˮƽ����

#define SSD_VER_PULSE_WIDTH		1		//��ֱ����
#define SSD_VER_BACK_PORCH		23		//��ֱǰ��
#define SSD_VER_FRONT_PORCH		22		//��ֱǰ��
//���¼����������Զ�����
#define SSD_HT	(SSD_HOR_RESOLUTION+SSD_HOR_BACK_PORCH+SSD_HOR_FRONT_PORCH)
#define SSD_HPS	(SSD_HOR_BACK_PORCH)
#define SSD_VT 	(SSD_VER_RESOLUTION+SSD_VER_BACK_PORCH+SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)
//
#define  GUI_CopyColor(color1, color2) 	*color1 = color2

void LCD_Init(void);													   	//��ʼ��
void LCD_DisplayOn(void);													//����ʾ
void LCD_DisplayOff(void);													//����ʾ
void LCD_Clear(u16 Color);	 												//����
void LCD_SetCursor(u16 Xpos, u16 Ypos);										//���ù��
void LCD_DrawPoint(u16 x,u16 y);											//����
void LCD_Fast_DrawPoint(u16 x,u16 y,u16 color);								//���ٻ���
u16  LCD_ReadPoint(u16 x,u16 y); 											//���� 
void Draw_Circle(u16 x0,u16 y0,u8 r);										//��Բ
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);							//����
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);		   				//������
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color);		   				//��䵥ɫ
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color);				//���ָ����ɫ
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode);						//��ʾһ���ַ�
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size);  						//��ʾһ������
void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode);				//��ʾ ����
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p);		//��ʾһ���ַ���,12/16����
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode);
void Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u8 len);
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue);
u16 LCD_ReadReg(u8 LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(u16 RGB_Code);		  
void LCD_Scan_Dir(u8 dir);							//������ɨ�跽��
void LCD_Display_Dir(u8 dir);						//������Ļ��ʾ����
void LCD_BackLightSet(u8 pwm);						//���ñ���
void LCD_EndXSet(u16 x);							//����X�յ�����
void LCD_Set_Window(u16 sx,u16 sy,u16 width,u16 height);//���ô���

void LCD_SetDisplayLayer(u16 layer);
void LCD_SetOperateLayer(u16 layer);
void JianTou_Up(u16 x, u16 y, u8 size, u16 color);			//�����ϼ�ͷ
void JianTou_Down(u16 x, u16 y, u8 size, u16 color);		//�����¼�ͷ
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color);
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color);
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color); 
void gui_draw_vline(u16 x0,u16 y0,u16 len,u16 color); 
void Show_GB32(u16 x,u16 y,u8 *font,u8 size,u8 mode);
void TriangularRegion(u16 x1, u16 x2, u16 y1, u16 x3, u16 y3, u16 color);
void Put_AnJian(u16 x, u16 y, u16 len, u16 wid);
void AnJianPressed(u16 x, u16 y, u16 len, u16 wid);
void  GUI_LoadPic(u32 x, u32 y, u8 *dat, u32 hno, u32 lno);
void  GUI_SetColor(u16 color1, u16 color2);
#endif  
	 
	 



