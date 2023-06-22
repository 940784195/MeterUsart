/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include	"rtc.h"
#include	"tftlcd.h"
#include	<string.h>	//用到函数strlen()
#include 	<stddef.h>

#define	 MAX_CMD_LEN	8	//指令�??大长度，RAM：XXX%
uint8_t	rxCompleted=RESET;	//HAL_UART_Receive_IT()接收是否完成，作为接收完成的标志�????
uint8_t	isUploadTime=0;	//默认不上传时间数�???

//定义新的空白数组用于处理数据
uint8_t RxBuffer[10];//定义数组 给到寄存�??
uint8_t ProcessBuffer[10];//定义数组用于数据处理
uint8_t	IndexBuffer=0;


uint16_t Times=0;
uint16_t A_CPU=0;
uint16_t A_RAM=0;
uint8_t CPUnum;

/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 57600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
	{


		rxCompleted=SET;	//接收完成,只有1个字�??
		__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE); //接收到数据后才开启IDLE中断

	}
}

void	on_UART_IDLE(UART_HandleTypeDef *huart)		//在串口IDLE时处�????
{
	//注意，这里不能使用函�??	__HAL_UART_GET_FLAG()，因为上位机连续�??5个字节，串口接收�??1个字节后虽然打开了IDLE中断�??
	//	但是因为后续连续发�?�数据，�??以IDLE中断挂起标志位并不会被置�??
	if(__HAL_UART_GET_IT_SOURCE(huart,UART_IT_IDLE) == RESET) //判断IDLE中断是否被开�??
		return;

	__HAL_UART_CLEAR_IDLEFLAG(huart); 	//清除IDLE标志
	__HAL_UART_DISABLE_IT(huart, UART_IT_IDLE); 	//禁止IDLE中断

	if (rxCompleted)	//接收
	{
		//接收到固定长度数据后使能UART_IT_IDLE中断，在UART_IT_IDLE中断里再次接�??
		//LCD_ShowStr(10, 320, "RxCompleted");
		ProcessBuffer[IndexBuffer]=RxBuffer[0];
		IndexBuffer++;
		if(ProcessBuffer[IndexBuffer]=='%'){IndexBuffer=0;}
		//if(strstr(ProcessBuffer,"CPU:"))//判断RxBuffer字符串中是否含有CPU�??

			char* failptr;//为失败不要的字符串部�?? 提供地址

			CPUnum = strtol(ProcessBuffer+4, &failptr, 10);//若输入的地址�??1，则偏移1位数�??
			LCD_ShowUint(50, LCD_CurY+LCD_SP15, CPUnum);
			LCD_ShowStr(10, LCD_CurY+LCD_SP15, ProcessBuffer);
	}


		if(  (strstr(ProcessBuffer,"CPU:")==NULL)  )
			{
			LCD_ShowStr(10, 400, (uint8_t *)"no CPU:");
			LCD_ShowStr(10, 400+LCD_SP15, ProcessBuffer);
			LCD_ShowStr(10, 400+2*LCD_SP15, (uint8_t *)ProcessBuffer);
			}

		LCD_ShowStr(10, 170, (uint8_t *)"Enter on_UART_IDLE Times");
		Times++;//记录进入空闲中断的时�??
		LCD_ShowUint(10, LCD_CurY+LCD_SP15,Times);
		rxCompleted=RESET;
		HAL_UART_Receive_IT(huart, RxBuffer, RX_CMD_LEN);	//再次启动串口接收

	}







/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
