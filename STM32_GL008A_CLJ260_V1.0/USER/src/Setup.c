/*
********************************************************************************
*  �� �� ����Setup.C
*
*  �ļ�������Ӳ����ʼ����غ���
*
*  ����оƬ��
*
*  �� �� �ˣ�
*
*  �� �� �ţ�
*
*  �޸ļ�¼��1.�޸���ʱ��������Ӧ��ͬʱ�ӡ�
********************************************************************************
*/
#include "stm32f10x.h"
#include "flash.h"
#include "SPI.h"
//��ʱus
void delay_us(u16 time)
{	
	u16 i;
 	for(;time>0;time--)
		for(i=10;i>0;i--);
}
//��ʱms
void delay_ms(u16 time)
{	
	u16 i;
 	for(;time>0;time--)
		for(i=12000;i>0;i--);
}

/* 
********************************************************************************
* �� �� ����USART_Configuration
* ��    �ܣ���ʼ��USART
* ��    ������
* ��    �أ���
* ˵    ����USART2 -- PA2(TX),  PA3(RX)         
* �׳��쳣��
* ��    �ߣ�fl
* �޸ļ�¼��
********************************************************************************
*/
void USART_Configuration(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    USART_InitTypeDef       USART_InitStructure;
    //USART_ClockInitTypeDef  USART_ClockInitStructure;           
		NVIC_InitTypeDef NVIC_InitStructure;
    /*USART2*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		//Usart2 NVIC ����
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//��ռ���ȼ�1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//�����ȼ�1
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
		NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
    
    USART_InitStructure.USART_BaudRate =9600;//���ڲ�����
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);   
}







//LCD FSMC���� 
void LCD_FSMCConfig(void)
{
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 
	FSMC_NORSRAMTimingInitTypeDef  writeTiming;	

	readWriteTiming.FSMC_AddressSetupTime = 0x01;	 //��ַ����ʱ�䣨ADDSET��Ϊ2��HCLK 1/36M=27ns
  readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //��ַ����ʱ�䣨ADDHLD��ģʽAδ�õ�	
  readWriteTiming.FSMC_DataSetupTime = 0x0f;		 // ���ݱ���ʱ��Ϊ16��HCLK,��ΪҺ������IC�Ķ����ݵ�ʱ���ٶȲ���̫�죬�����1289���IC��
  readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
  readWriteTiming.FSMC_CLKDivision = 0x00;
  readWriteTiming.FSMC_DataLatency = 0x00;
  readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //ģʽA 
    

	writeTiming.FSMC_AddressSetupTime = 0x00;	 //��ַ����ʱ�䣨ADDSET��Ϊ1��HCLK  
  writeTiming.FSMC_AddressHoldTime = 0x00;	 //��ַ����ʱ�䣨A		
  writeTiming.FSMC_DataSetupTime = 0x03;		 ////���ݱ���ʱ��Ϊ4��HCLK	
  writeTiming.FSMC_BusTurnAroundDuration = 0x00;
  writeTiming.FSMC_CLKDivision = 0x00;
  writeTiming.FSMC_DataLatency = 0x00;
  writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //ģʽA 

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;  
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //��дʱ��
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //дʱ��
  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  
  /* BANK 4 (of NOR/SRAM Bank 1~4) is enabled */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
		
}
// FSMC����
void LCD_CtrlLinesConfig(void)
{

	RCC->AHBENR|=1<<8;      //ʹ��FSMCʱ��	  
	RCC->APB2ENR|=1<<3;     //ʹ��PORTBʱ��
	RCC->APB2ENR|=1<<5;     //ʹ��PORTDʱ��
	RCC->APB2ENR|=1<<6;     //ʹ��PORTEʱ��
 	RCC->APB2ENR|=1<<8;     //ʹ��PORTGʱ��	 
	GPIOB->CRL&=0XFFFFFFF0;	//PB0 ������� ��λ����
	GPIOB->CRL|=0X00000003;	  
	//PORTD����������� 	
	GPIOD->CRH&=0X00FFF000;
	GPIOD->CRH|=0XBB000BBB; 
	GPIOD->CRL&=0XFF00FF00;
	GPIOD->CRL|=0X00BB00BB;   	 
	//PORTE����������� 	
	GPIOE->CRH&=0X00000000;
	GPIOE->CRH|=0XBBBBBBBB; 
	GPIOE->CRL&=0X0FFFFFFF;
	GPIOE->CRL|=0XB0000000;    	    	 											 
	//PORTG12�����������  	    	 											 
	GPIOG->CRH&=0XFFF0FFFF;
	GPIOG->CRH|=0X000B0000; 
	GPIOG->CRL&=0XFFFFFFF0;//PG0->RS
	GPIOG->CRL|=0X0000000B;
}

/* 
********************************************************************************
* �� �� ����RCC_Configuration
* ��    �ܣ�ͳ��Χ����ʱ������
* ��    ������
* ��    �أ���
* ˵    ������
* �׳��쳣��
* ��    �ߣ�fl
* �޸ļ�¼��
********************************************************************************
*/
void RCC_Configuration(void)
{
// 	//����CRCʱ�� ����emWIN
//   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC,ENABLE); 
	/* Enable GPIOA,GPIOB,GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */	 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC |RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
                         RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |RCC_APB2Periph_AFIO, ENABLE);
	/* Enable FSMC */	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	/* Enable spi */	
	RCC_APB1PeriphClockCmd(	RCC_APB2Periph_SPI1|RCC_APB1Periph_SPI2,ENABLE );//SPI1.SPI12ʱ��ʹ�� 	
	/* ��������2 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
}



/* 
********************************************************************************
* �� �� ����NVIC_Configuration
* ��    �ܣ�ϵͳ�ж��������ȼ�����
* ��    ������
* ��    �أ���
* ˵    ����
* �׳��쳣��
* ��    �ߣ�fl
* �޸ļ�¼��
********************************************************************************
*/
void NVIC_Configuration(void)
{	

}
//����NVIC����
//NVIC_Group:NVIC���� 0~4 �ܹ�5�� 		   
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//ȡ����λ
	temp1<<=8;
	temp=SCB->AIRCR;  //��ȡ��ǰ������
	temp&=0X0000F8FF; //�����ǰ����
	temp|=0X05FA0000; //д��Կ��
	temp|=temp1;	   
	SCB->AIRCR=temp;  //���÷���	    	  				   
}
//����NVIC 
//NVIC_PreemptionPriority:��ռ���ȼ�
//NVIC_SubPriority       :��Ӧ���ȼ�
//NVIC_Channel           :�жϱ��
//NVIC_Group             :�жϷ��� 0~4
//ע�����ȼ����ܳ����趨����ķ�Χ!����������벻���Ĵ���
//�黮��:
//��0:0λ��ռ���ȼ�,4λ��Ӧ���ȼ�
//��1:1λ��ռ���ȼ�,3λ��Ӧ���ȼ�
//��2:2λ��ռ���ȼ�,2λ��Ӧ���ȼ�
//��3:3λ��ռ���ȼ�,1λ��Ӧ���ȼ�
//��4:4λ��ռ���ȼ�,0λ��Ӧ���ȼ�
//NVIC_SubPriority��NVIC_PreemptionPriority��ԭ����,��ֵԽС,Խ����	   
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 
{ 
	u32 temp;	
	MY_NVIC_PriorityGroupConfig(NVIC_Group);//���÷���
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);	  
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;//ȡ����λ  
	if(NVIC_Channel<32)NVIC->ISER[0]|=1<<NVIC_Channel;//ʹ���ж�λ(Ҫ����Ļ�,�෴������OK)
	else NVIC->ISER[1]|=1<<(NVIC_Channel-32);    
	NVIC->IP[NVIC_Channel]|=temp<<4;//������Ӧ���ȼ����������ȼ�   	    	  				   
}

/* 
********************************************************************************
* �� �� ����GPIO_Configuration
* ��    �ܣ���ʼ��IO��
* ��    ������
* ��    �أ���
* ˵    ��������PB6 ���õĳ�ʼ�� �������ĳ�ʼ�� PB8������
* �׳��쳣��
* ��    �ߣ�fl
* �޸ļ�¼��
********************************************************************************
*/
void GPIO_Configuration(void)
{

	//PB6(����J8)���õĳ�ʼ�� PB8������	
  GPIO_InitTypeDef  GPIO_InitStructure; 	
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;	//�˿�����PB5 PB6 PB7 PB8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);				//�����趨������ʼ��GPIOB6
	GPIO_ResetBits(GPIOB,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8);						 
	
	//PE5 ����DEHSҺ����ؼ̵�������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	//�˿�����PE5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);				
	GPIO_ResetBits(GPIOE,GPIO_Pin_5);						 //PE5

}


/* 
********************************************************************************
* �� �� ����RTC_Configuration
* ��    �ܣ�ʵʱʱ��250ms�ж�һ��
* ��    ������
* ��    �أ���
* ˵    ����
* �׳��쳣��
* ��    �ߣ�fl
* �޸ļ�¼��
********************************************************************************
*/
void RTC_Configuration(void)
{

}



/* 
********************************************************************************
* �� �� ����
* ��    �ܣ�
* ��    ����
* ��    �أ� 
* ˵    ����
* �׳��쳣��
* ��    �ߣ�023
* �޸ļ�¼��
********************************************************************************
*/
/*
void ADC_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    //PB0--TBT2--�����2(4.8V)
    //PB1--TBT1--�����1(3.6V)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    DMA_DeInit(DMA1_Channel1);
    //DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
    //DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADCConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 2;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    //Enable DMA1 channel1 
    DMA_Cmd(DMA1_Channel1, ENABLE);

    //ADC1 configuration 
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 2;
    ADC_Init(ADC1, &ADC_InitStructure);

    //ADC1 regular channel,8,9 configuration 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_71Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_71Cycles5);

    //Enable ADC1 DMA 
    ADC_DMACmd(ADC1, ENABLE);

    //Enable ADC1 
    ADC_Cmd(ADC1, ENABLE);

    //Enable ADC1 reset calibaration register 
    ADC_ResetCalibration(ADC1);
    //Check the end of ADC1 reset calibration register 
    while(ADC_GetResetCalibrationStatus(ADC1));

    //Start ADC1 calibaration 
    ADC_StartCalibration(ADC1);
    //Check the end of ADC1 calibration 
    while(ADC_GetCalibrationStatus(ADC1));

    //Start ADC1 Software Conversion 
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


*/
//��ʼ��ADC
//�������ǽ��Թ���ͨ��Ϊ��
//����Ĭ�Ͻ�����ͨ��1																	   
void  Adc_Init(void)
{    
	//�ȳ�ʼ��IO��
 	RCC->APB2ENR|=1<<2;    //ʹ��PORTA��ʱ�� 
	GPIOA->CRL&=0XFFFFFF0F;//PA1 anolog����
	RCC->APB2ENR|=1<<4;    //ʹ��PORTC��ʱ�� 
	GPIOC->CRL&=0XFFFFFFF0;//PC0 anolog����
	//ͨ��10/11����			 
	RCC->APB2ENR|=1<<9;    //ADC1ʱ��ʹ��	  
	RCC->APB2RSTR|=1<<9;   //ADC1��λ
	RCC->APB2RSTR&=~(1<<9);//��λ����	    
	RCC->CFGR&=~(3<<14);   //��Ƶ��������	
	//SYSCLK/DIV2=12M ADCʱ������Ϊ12M,ADC���ʱ�Ӳ��ܳ���14M!
	//���򽫵���ADC׼ȷ���½�! 
	RCC->CFGR|=2<<14;      	 
	ADC1->CR1&=0XF0FFFF;   //����ģʽ����
	ADC1->CR1|=0<<16;      //��������ģʽ  
	ADC1->CR1&=~(1<<8);    //��ɨ��ģʽ	  
	ADC1->CR2&=~(1<<1);    //����ת��ģʽ
	ADC1->CR2&=~(7<<17);	   
	ADC1->CR2|=7<<17;	   //�������ת��  
	ADC1->CR2|=1<<20;      //ʹ�����ⲿ����(SWSTART)!!!	����ʹ��һ���¼�������
	ADC1->CR2&=~(1<<11);   //�Ҷ���	 
	ADC1->SQR1&=~(0XF<<20);
	ADC1->SQR1|=0<<20;     //1��ת���ڹ��������� Ҳ����ֻת����������1 			   
	//����ͨ��1�Ĳ���ʱ��
	ADC1->SMPR1&=~(7<<0);  //ͨ��10����ʱ�����	  
 	ADC1->SMPR1|=7<<0;     //ͨ��10  239.5����,��߲���ʱ�������߾�ȷ��
	ADC1->SMPR2&=~(7<<3);  //ͨ��1����ʱ�����	  
 	ADC1->SMPR2|=7<<3;     //ͨ��1  239.5����,��߲���ʱ�������߾�ȷ��	 
	ADC1->CR2|=1<<0;	   	//����ADת����	 
	ADC1->CR2|=1<<3;       //ʹ�ܸ�λУ׼  
	while(ADC1->CR2&1<<3); //�ȴ�У׼���� 			 
    //��λ��������ò���Ӳ���������У׼�Ĵ�������ʼ�����λ��������� 		 
	ADC1->CR2|=1<<2;        //����ADУ׼	   
	while(ADC1->CR2&1<<2);  //�ȴ�У׼����
	//��λ����������Կ�ʼУ׼������У׼����ʱ��Ӳ�����  
}	
//���ADCֵ
//ch:ͨ��ֵ 0~16
//����ֵ:ת�����
u16 Get_Adc(u8 ch)   
{
	//����ת������	  		 
	ADC1->SQR3&=0XFFFFFFE0;//��������1 ͨ��ch
	ADC1->SQR3|=ch;		  			    
	ADC1->CR2|=1<<22;       //��������ת��ͨ�� 
	while(!(ADC1->SR&1<<1));//�ȴ�ת������	 	   
	return ADC1->DR;		//����adcֵ	
}
u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{		
		temp_val+=Get_Adc(ch);
		delay_us(50);		
}
	return temp_val/times;
} 
void TIM4_Int_Init(void)	//��ʱ1ms
{
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

	TIM_TimeBaseInitStructure.TIM_Prescaler=71;   
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;	  
	TIM_TimeBaseInitStructure.TIM_Period=999;		 
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM4,ENABLE);	

}
//PB7�������õļ̵����ź�
void starttest(void)
{	
 	GPIO_SetBits(GPIOB,GPIO_Pin_6);						 //PB6 �����
}
//PB7�������õļ̵����ź�
void stoptest(void)
{	
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);						 //PB6 �����
}


/* 
********************************************************************************
* �� �� ����InitHardware
* ��    �ܣ���ʼ��ϵͳӲ��
* ��    ������
* ��    �أ���
* ˵    ����
* �׳��쳣��
* ��    �ߣ�023
* �޸ļ�¼��
********************************************************************************
*/
void InitHardware(void)
{  		
		RCC_Configuration();		
		GPIO_Configuration();
		LCD_FSMCConfig();	
		LCD_CtrlLinesConfig();
		TIM4_Int_Init();
		SPI_Flash_Init();
		USART_Configuration();
}



