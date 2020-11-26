/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "Setup.h"
#include "Serial.h"
u16 OS_TimeMS;


extern u16 pressADvalue;//AD值
extern u16 press_zeroAD;//零点AD
float press_k;//曲线斜率
float press_c;//曲线斜率
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}
void TIM4_IRQHandler(void)
{
	if(U2DS.RecRun)   //;串口2正在接收字节
    { 
        U2DS.RBet2Byte++;      //;记录串口2接收两个字节之间的时间距离
    }
	if(U2DS.TBusy)  				  //;串口2发送1个字节以后的延时时间
    { 
        U2DS.TBusy--; 
    }	
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)!=RESET)//检查指定的定时器中断是否发生
	{	
		OS_TimeMS++;
		if(OS_TimeMS%13000==0)//11S
		{	
			OS_TimeMS=0;	
 			RTC_13000MS=1;
		}
		else if(OS_TimeMS%12000==0)//10S
		{			
 			RTC_12000MS=1;
		}
		else if(OS_TimeMS%5000==0)//2.5S
		{				
 			RTC_2500MS=1;
		}
		else if(OS_TimeMS%500==0)//500MS
		{			
 			RTC_500MS=1;
		}
		else if(OS_TimeMS%250==0)//250MS
		{			
 			RTC_250MS=1;
		}				
	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);//清除中断标志位，方便下次进入
}

/*USART2_IRQHandler*/
void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        ReceiveByte_USART2();
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);   //;清接收的中断标志
    }
    if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {
        SendByte_USART2();
        USART_ClearITPendingBit(USART2, USART_IT_TXE);   //;清发送的中断标志
    }
}
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 



/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
