/*
********************************************************************************
*  文 件 名：Setup.C
*
*  文件描述：硬件初始化相关函数
*
*  所用芯片：
*
*  创 建 人：
*
*  版 本 号：
*
*  修改记录：1.修改延时函数以适应不同时钟。
********************************************************************************
*/
#include "stm32f10x.h"
#include "flash.h"
#include "SPI.h"
//延时us
void delay_us(u16 time)
{	
	u16 i;
 	for(;time>0;time--)
		for(i=10;i>0;i--);
}
//延时ms
void delay_ms(u16 time)
{	
	u16 i;
 	for(;time>0;time--)
		for(i=12000;i>0;i--);
}

/* 
********************************************************************************
* 函 数 名：USART_Configuration
* 功    能：初始化USART
* 参    数：无
* 返    回：无
* 说    明：USART2 -- PA2(TX),  PA3(RX)         
* 抛出异常：
* 作    者：fl
* 修改记录：
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
		//Usart2 NVIC 配置
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级1
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
    
    USART_InitStructure.USART_BaudRate =9600;//串口波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);   
}







//LCD FSMC配置 
void LCD_FSMCConfig(void)
{
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 
	FSMC_NORSRAMTimingInitTypeDef  writeTiming;	

	readWriteTiming.FSMC_AddressSetupTime = 0x01;	 //地址建立时间（ADDSET）为2个HCLK 1/36M=27ns
  readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（ADDHLD）模式A未用到	
  readWriteTiming.FSMC_DataSetupTime = 0x0f;		 // 数据保存时间为16个HCLK,因为液晶驱动IC的读数据的时候，速度不能太快，尤其对1289这个IC。
  readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
  readWriteTiming.FSMC_CLKDivision = 0x00;
  readWriteTiming.FSMC_DataLatency = 0x00;
  readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 
    

	writeTiming.FSMC_AddressSetupTime = 0x00;	 //地址建立时间（ADDSET）为1个HCLK  
  writeTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（A		
  writeTiming.FSMC_DataSetupTime = 0x03;		 ////数据保存时间为4个HCLK	
  writeTiming.FSMC_BusTurnAroundDuration = 0x00;
  writeTiming.FSMC_CLKDivision = 0x00;
  writeTiming.FSMC_DataLatency = 0x00;
  writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 

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
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //读写时序
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //写时序
  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  
  /* BANK 4 (of NOR/SRAM Bank 1~4) is enabled */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
		
}
// FSMC连接
void LCD_CtrlLinesConfig(void)
{

	RCC->AHBENR|=1<<8;      //使能FSMC时钟	  
	RCC->APB2ENR|=1<<3;     //使能PORTB时钟
	RCC->APB2ENR|=1<<5;     //使能PORTD时钟
	RCC->APB2ENR|=1<<6;     //使能PORTE时钟
 	RCC->APB2ENR|=1<<8;     //使能PORTG时钟	 
	GPIOB->CRL&=0XFFFFFFF0;	//PB0 推挽输出 复位引脚
	GPIOB->CRL|=0X00000003;	  
	//PORTD复用推挽输出 	
	GPIOD->CRH&=0X00FFF000;
	GPIOD->CRH|=0XBB000BBB; 
	GPIOD->CRL&=0XFF00FF00;
	GPIOD->CRL|=0X00BB00BB;   	 
	//PORTE复用推挽输出 	
	GPIOE->CRH&=0X00000000;
	GPIOE->CRH|=0XBBBBBBBB; 
	GPIOE->CRL&=0X0FFFFFFF;
	GPIOE->CRL|=0XB0000000;    	    	 											 
	//PORTG12复用推挽输出  	    	 											 
	GPIOG->CRH&=0XFFF0FFFF;
	GPIOG->CRH|=0X000B0000; 
	GPIOG->CRL&=0XFFFFFFF0;//PG0->RS
	GPIOG->CRL|=0X0000000B;
}

/* 
********************************************************************************
* 函 数 名：RCC_Configuration
* 功    能：统外围器件时钟配置
* 参    数：无
* 返    回：无
* 说    明：无
* 抛出异常：
* 作    者：fl
* 修改记录：
********************************************************************************
*/
void RCC_Configuration(void)
{
// 	//开启CRC时钟 启动emWIN
//   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC,ENABLE); 
	/* Enable GPIOA,GPIOB,GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */	 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC |RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
                         RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |RCC_APB2Periph_AFIO, ENABLE);
	/* Enable FSMC */	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	/* Enable spi */	
	RCC_APB1PeriphClockCmd(	RCC_APB2Periph_SPI1|RCC_APB1Periph_SPI2,ENABLE );//SPI1.SPI12时钟使能 	
	/* 开启串口2 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
}



/* 
********************************************************************************
* 函 数 名：NVIC_Configuration
* 功    能：系统中断向量优先级配置
* 参    数：无
* 返    回：无
* 说    明：
* 抛出异常：
* 作    者：fl
* 修改记录：
********************************************************************************
*/
void NVIC_Configuration(void)
{	

}
//设置NVIC分组
//NVIC_Group:NVIC分组 0~4 总共5组 		   
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//取后三位
	temp1<<=8;
	temp=SCB->AIRCR;  //读取先前的设置
	temp&=0X0000F8FF; //清空先前分组
	temp|=0X05FA0000; //写入钥匙
	temp|=temp1;	   
	SCB->AIRCR=temp;  //设置分组	    	  				   
}
//设置NVIC 
//NVIC_PreemptionPriority:抢占优先级
//NVIC_SubPriority       :响应优先级
//NVIC_Channel           :中断编号
//NVIC_Group             :中断分组 0~4
//注意优先级不能超过设定的组的范围!否则会有意想不到的错误
//组划分:
//组0:0位抢占优先级,4位响应优先级
//组1:1位抢占优先级,3位响应优先级
//组2:2位抢占优先级,2位响应优先级
//组3:3位抢占优先级,1位响应优先级
//组4:4位抢占优先级,0位响应优先级
//NVIC_SubPriority和NVIC_PreemptionPriority的原则是,数值越小,越优先	   
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 
{ 
	u32 temp;	
	MY_NVIC_PriorityGroupConfig(NVIC_Group);//设置分组
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);	  
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;//取低四位  
	if(NVIC_Channel<32)NVIC->ISER[0]|=1<<NVIC_Channel;//使能中断位(要清除的话,相反操作就OK)
	else NVIC->ISER[1]|=1<<(NVIC_Channel-32);    
	NVIC->IP[NVIC_Channel]|=temp<<4;//设置响应优先级和抢断优先级   	    	  				   
}

/* 
********************************************************************************
* 函 数 名：GPIO_Configuration
* 功    能：初始化IO口
* 参    数：无
* 返    回：无
* 说    明：控制PB6 气泵的初始化 蜂鸣器的初始化 PB8蜂鸣器
* 抛出异常：
* 作    者：fl
* 修改记录：
********************************************************************************
*/
void GPIO_Configuration(void)
{

	//PB6(控制J8)气泵的初始化 PB8蜂鸣器	
  GPIO_InitTypeDef  GPIO_InitStructure; 	
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;	//端口配置PB5 PB6 PB7 PB8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);				//根据设定参数初始化GPIOB6
	GPIO_ResetBits(GPIOB,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8);						 
	
	//PE5 控制DEHS液的相关继电器动作
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	//端口配置PE5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//IO口速度为50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);				
	GPIO_ResetBits(GPIOE,GPIO_Pin_5);						 //PE5

}


/* 
********************************************************************************
* 函 数 名：RTC_Configuration
* 功    能：实时时钟250ms中断一次
* 参    数：无
* 返    回：无
* 说    明：
* 抛出异常：
* 作    者：fl
* 修改记录：
********************************************************************************
*/
void RTC_Configuration(void)
{

}



/* 
********************************************************************************
* 函 数 名：
* 功    能：
* 参    数：
* 返    回： 
* 说    明：
* 抛出异常：
* 作    者：023
* 修改记录：
********************************************************************************
*/
/*
void ADC_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    //PB0--TBT2--检测电池2(4.8V)
    //PB1--TBT1--检测电池1(3.6V)
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
//初始化ADC
//这里我们仅以规则通道为例
//我们默认仅开启通道1																	   
void  Adc_Init(void)
{    
	//先初始化IO口
 	RCC->APB2ENR|=1<<2;    //使能PORTA口时钟 
	GPIOA->CRL&=0XFFFFFF0F;//PA1 anolog输入
	RCC->APB2ENR|=1<<4;    //使能PORTC口时钟 
	GPIOC->CRL&=0XFFFFFFF0;//PC0 anolog输入
	//通道10/11设置			 
	RCC->APB2ENR|=1<<9;    //ADC1时钟使能	  
	RCC->APB2RSTR|=1<<9;   //ADC1复位
	RCC->APB2RSTR&=~(1<<9);//复位结束	    
	RCC->CFGR&=~(3<<14);   //分频因子清零	
	//SYSCLK/DIV2=12M ADC时钟设置为12M,ADC最大时钟不能超过14M!
	//否则将导致ADC准确度下降! 
	RCC->CFGR|=2<<14;      	 
	ADC1->CR1&=0XF0FFFF;   //工作模式清零
	ADC1->CR1|=0<<16;      //独立工作模式  
	ADC1->CR1&=~(1<<8);    //非扫描模式	  
	ADC1->CR2&=~(1<<1);    //单次转换模式
	ADC1->CR2&=~(7<<17);	   
	ADC1->CR2|=7<<17;	   //软件控制转换  
	ADC1->CR2|=1<<20;      //使用用外部触发(SWSTART)!!!	必须使用一个事件来触发
	ADC1->CR2&=~(1<<11);   //右对齐	 
	ADC1->SQR1&=~(0XF<<20);
	ADC1->SQR1|=0<<20;     //1个转换在规则序列中 也就是只转换规则序列1 			   
	//设置通道1的采样时间
	ADC1->SMPR1&=~(7<<0);  //通道10采样时间清空	  
 	ADC1->SMPR1|=7<<0;     //通道10  239.5周期,提高采样时间可以提高精确度
	ADC1->SMPR2&=~(7<<3);  //通道1采样时间清空	  
 	ADC1->SMPR2|=7<<3;     //通道1  239.5周期,提高采样时间可以提高精确度	 
	ADC1->CR2|=1<<0;	   	//开启AD转换器	 
	ADC1->CR2|=1<<3;       //使能复位校准  
	while(ADC1->CR2&1<<3); //等待校准结束 			 
    //该位由软件设置并由硬件清除。在校准寄存器被初始化后该位将被清除。 		 
	ADC1->CR2|=1<<2;        //开启AD校准	   
	while(ADC1->CR2&1<<2);  //等待校准结束
	//该位由软件设置以开始校准，并在校准结束时由硬件清除  
}	
//获得ADC值
//ch:通道值 0~16
//返回值:转换结果
u16 Get_Adc(u8 ch)   
{
	//设置转换序列	  		 
	ADC1->SQR3&=0XFFFFFFE0;//规则序列1 通道ch
	ADC1->SQR3|=ch;		  			    
	ADC1->CR2|=1<<22;       //启动规则转换通道 
	while(!(ADC1->SR&1<<1));//等待转换结束	 	   
	return ADC1->DR;		//返回adc值	
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
void TIM4_Int_Init(void)	//定时1ms
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
//PB7控制气泵的继电器信号
void starttest(void)
{	
 	GPIO_SetBits(GPIOB,GPIO_Pin_6);						 //PB6 输出高
}
//PB7控制气泵的继电器信号
void stoptest(void)
{	
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);						 //PB6 输出低
}


/* 
********************************************************************************
* 函 数 名：InitHardware
* 功    能：初始化系统硬件
* 参    数：无
* 返    回：无
* 说    明：
* 抛出异常：
* 作    者：023
* 修改记录：
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



