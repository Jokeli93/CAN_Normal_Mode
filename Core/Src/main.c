/*
 * main.c
 *
 *  Created on: Jul 13, 2026
 *      Author: Joelikane
 */


#include "main.h"

void Error_Handler(void);
void SystemClock_Config_HSE(uint8_t clock_freq);
void GPIO_Init(void);
void UART2_Init(void);
void TIM6_Init(void);
void CAN1_Init(void);
void CAN1_Filter_Config(void);
void CAN1_TX(void);
void CAN1_RX(void);

UART_HandleTypeDef huart2;
CAN_HandleTypeDef hcan1;
TIM_HandleTypeDef htimer6;


int main(void)
{

	HAL_Init();

	SystemClock_Config_HSE(SYS_CLOCK_FREQ_50_MHZ);

	GPIO_Init();

	UART2_Init();

	TIM6_Init();

	//start the timer in interrupt mode
	//HAL_TIM_Base_Start_IT(&htimer6);

	//move CAN peripheral from sleep mode in to initialization mode
	CAN1_Init();

	//CAN filter configuration
	CAN1_Filter_Config();

	//Enable interrupts for the CAN1 peripheral
	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_BUSOFF))
		Error_Handler();

	//move CAN peripheral from initialization mode in to normal mode
	if(HAL_CAN_Start(&hcan1) != HAL_OK)
		Error_Handler();

	//CAN1_TX();

	//CAN1_RX();

	while(1);

	return 0;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config_HSE(uint8_t clock_freq )
{
	RCC_OscInitTypeDef Osc_Init = {0};
	RCC_ClkInitTypeDef Clock_Init = {0};
	uint8_t flash_latency = 0;

	/* --- NEU: Schreibschutz für LSE/Backup-Domain aufheben --- */
	//__HAL_RCC_PWR_CLK_ENABLE();      // Takt für Power-Interface einschalten
	//HAL_PWR_EnableBkUpAccess();      // Zugriff auf Backup-Domain (LSE) freischalten
	/* -------------------------------------------------------- */

	Osc_Init.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_HSI;
	Osc_Init.HSEState = RCC_HSE_ON;
	//Osc_Init.LSEState = RCC_LSE_ON;
	Osc_Init.HSIState = RCC_HSI_ON;
	Osc_Init.HSICalibrationValue = 16;
	Osc_Init.PLL.PLLState = RCC_PLL_ON;
	Osc_Init.PLL.PLLSource = RCC_PLLSOURCE_HSE;

	switch(clock_freq) {
		case SYS_CLOCK_FREQ_50_MHZ:
			Osc_Init.PLL.PLLM = 4;
			Osc_Init.PLL.PLLN = 50;
			Osc_Init.PLL.PLLP = RCC_PLLP_DIV2; // 8MHz / 4 * 50 /2 = 50 MHz
			Osc_Init.PLL.PLLQ = 2;

			Clock_Init.ClockType = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK |
								   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
			Clock_Init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			Clock_Init.AHBCLKDivider = RCC_SYSCLK_DIV1;
			Clock_Init.APB1CLKDivider = RCC_HCLK_DIV2; // APB1 = 25 MHz (Timer = 50 MHz)
			Clock_Init.APB2CLKDivider = RCC_HCLK_DIV1;

			flash_latency = FLASH_LATENCY_1; // 1 wait state for 50MHz
			break;

		case SYS_CLOCK_FREQ_84_MHZ:
			Osc_Init.PLL.PLLM = 4;
			Osc_Init.PLL.PLLN = 84;
			Osc_Init.PLL.PLLP = RCC_PLLP_DIV2; // 8MHz / 4 * 84 /2 = 84 MHz
			Osc_Init.PLL.PLLQ = 2;

			Clock_Init.ClockType = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK |
								   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
			Clock_Init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			Clock_Init.AHBCLKDivider = RCC_SYSCLK_DIV1;
			Clock_Init.APB1CLKDivider = RCC_HCLK_DIV2;
			Clock_Init.APB2CLKDivider = RCC_HCLK_DIV1;

			flash_latency = FLASH_LATENCY_2; //2 wait states for 84MHz
			break;

		case SYS_CLOCK_FREQ_120_MHZ:
			Osc_Init.PLL.PLLM = 4;
			Osc_Init.PLL.PLLN = 120;
			Osc_Init.PLL.PLLP = RCC_PLLP_DIV2; // 8MHz / 4 * 120 /2 = 120 MHz
			Osc_Init.PLL.PLLQ = 2;

			Clock_Init.ClockType = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK |
								   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
			Clock_Init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
			Clock_Init.AHBCLKDivider = RCC_SYSCLK_DIV1;
			Clock_Init.APB1CLKDivider = RCC_HCLK_DIV4;
			Clock_Init.APB2CLKDivider = RCC_HCLK_DIV2;

			flash_latency = FLASH_LATENCY_3; //3 wait state for 120MHz
			break;

		default:
			return ;
	}

	if (HAL_RCC_OscConfig(&Osc_Init) != HAL_OK)
	{
	  Error_Handler();
	}

	if (HAL_RCC_ClockConfig(&Clock_Init, flash_latency) != HAL_OK)
	{
		Error_Handler();
	}

	/*Configure the systick timer interrupt frequency (for every 1 ms) */
	uint32_t hclk_freq = HAL_RCC_GetHCLKFreq();
	HAL_SYSTICK_Config(hclk_freq/1000);

	/**Configure the Systick
	*/
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void UART2_Init(void)
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;

	if(HAL_UART_Init(&huart2) != HAL_OK)
	{
		//there is a problem
		Error_Handler();
	}
}

void CAN1_Init(void)
{
	//Settings related to the CAN controller
	hcan1.Instance = CAN1;
	hcan1.Init.Mode = CAN_MODE_NORMAL;
	hcan1.Init.AutoBusOff = DISABLE;
	hcan1.Init.AutoRetransmission = ENABLE;
	hcan1.Init.AutoWakeUp = DISABLE;
	hcan1.Init.ReceiveFifoLocked = DISABLE;
	hcan1.Init.TimeTriggeredMode = DISABLE;
	hcan1.Init.TransmitFifoPriority = DISABLE;

	//Settings related to the CAN bit timings
	hcan1.Init.Prescaler = 5; // 25 MHz / 5 = 5 MHz CAN-Takt
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
	//hcan1.Init.TimeSeg1 = CAN_BS1_8TQ;
	hcan1.Init.TimeSeg1 = CAN_BS1_7TQ;
	//hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
	hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
	/*---------------------------------------------------------*/
	// Total-TQ = 1 (Sync) + 7 (BS1) + 2 (BS2) = 10 TQ
	// Sample Point = (1 + 7) / 10 = 80% (perfect standard value!)
	//Baudrate = 5 MHz / 10 TQ = 500 Kbit/s

	//Initialization of CAN1 peripheral
	if(HAL_CAN_Init(&hcan1) != HAL_OK)
		Error_Handler();

}

void CAN1_Filter_Config(void)
{
	CAN_FilterTypeDef can1_filter_init;

	//Settings related to the CAN filter
	can1_filter_init.FilterActivation = ENABLE;
	can1_filter_init.FilterBank = 0;
	can1_filter_init.FilterFIFOAssignment = CAN_RX_FIFO0;
	can1_filter_init.FilterIdHigh = 0x0000;
	can1_filter_init.FilterIdLow = 0x0000;
	can1_filter_init.FilterMaskIdHigh = 0x0000;
	can1_filter_init.FilterMaskIdLow = 0x0000;
	can1_filter_init.FilterMode = CAN_FILTERMODE_IDMASK;
	can1_filter_init.FilterScale = CAN_FILTERSCALE_32BIT;

	//Filter initialization
	if(HAL_CAN_ConfigFilter(&hcan1, &can1_filter_init) != HAL_OK)
		Error_Handler();
}

/*
 * Timer configuration with a time base of 1 sec
 */
void TIM6_Init(void)
{
	htimer6.Instance = TIM6;
	htimer6.Init.Prescaler = 50000 -1; // 50MHz / 50.000 = 1000Hz takt
	htimer6.Init.Period = 1000 - 1;   // 1000 Ticks for 1000 HZ = 1 sec.

	if(HAL_TIM_Base_Init(&htimer6) != HAL_OK)
	{
		Error_Handler();
	}
}

void GPIO_Init(void)
{
	GPIO_InitTypeDef led_gpio = {0};

	__HAL_RCC_GPIOD_CLK_ENABLE();

	led_gpio.Mode = GPIO_MODE_OUTPUT_PP;
	led_gpio.Pull = GPIO_NOPULL;
	led_gpio.Speed = GPIO_SPEED_FREQ_LOW;
	led_gpio.Pin = GPIO_PIN_12;

	HAL_GPIO_Init(GPIOD, &led_gpio);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

}

void Error_Handler(void)
{
	while(1);
}
