/*
 * msp.c
 *
 *  Created on: Jul 13, 2026
 *      Author: Joelikane
 */


#include "main.h"

void HAL_MspInit(void)
{
  //Here will do low  level processor specific inits

	//1. Set up the priority grouping of the ARM Cortex Mx processor
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	//2. Enable the required system exeptions of the ARM Cortex Mx Processor
	SCB->SHCSR |= (0x7 << 16); //usg  fault, memory fault and bus fault system exceptions

	//3. Configure the priority for the system exeptions
	HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
	HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
	HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef gpio_uart;

	//here we do the low level Inits of the USART2 peripheral

	//1. enable the clock of the USART2 and GPIOA peripherals
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//2. Pin muxing configurations
	gpio_uart.Mode = GPIO_MODE_AF_PP;
	gpio_uart.Pull = GPIO_PULLUP;
	gpio_uart.Speed = GPIO_SPEED_FREQ_LOW;
	gpio_uart.Alternate = GPIO_AF7_USART2;

	gpio_uart.Pin = GPIO_PIN_2;
	HAL_GPIO_Init(GPIOA, &gpio_uart);//UART2_TX

	gpio_uart.Pin = GPIO_PIN_3;
	HAL_GPIO_Init(GPIOA, &gpio_uart);//UART_RX

	//3.Enable the IRQ and set up the priority (NVIC settings)
	HAL_NVIC_EnableIRQ(USART2_IRQn);
	HAL_NVIC_SetPriority(USART2_IRQn, 15, 0);
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
	GPIO_InitTypeDef gpio_can = {0};

	//Enable the clock of the CAN1 and GPIOD peripherals
	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*CAN1 GPIO configuration
	PD0 --> CAN1_RX
	PD1 --> CAN1_TX
	*/
	gpio_can.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	gpio_can.Mode = GPIO_MODE_AF_PP;
	gpio_can.Pull = GPIO_NOPULL;
	gpio_can.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_can.Alternate = GPIO_AF9_CAN1;
	HAL_GPIO_Init(GPIOD, &gpio_can);

	//Enable the IRQ and set up the priority (NVIC settings)
	HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
	HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
	HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
	HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);

	HAL_NVIC_SetPriority(CAN1_TX_IRQn, 15, 0);
	HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 15, 0);
	HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 15, 0);
	HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 15, 0);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htimer)
{
	//1.Enable the clock for the TIM6 and GPIO peripheral
	__HAL_RCC_TIM6_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	//2. Enable the IRQ of TIM6
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

	//3. setup the priority for TIM6
	HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 15, 0);
}
