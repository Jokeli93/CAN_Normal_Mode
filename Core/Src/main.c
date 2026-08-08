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

void LED_Manage_Output(uint8_t led_number);
void send_response(uint32_t Id);
uint8_t req_counter = 0;
uint8_t led_no = 0;

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
	//hcan1.Init.AutoBusOff = DISABLE;
	hcan1.Init.AutoBusOff = ENABLE;
	hcan1.Init.AutoRetransmission = ENABLE;
	hcan1.Init.AutoWakeUp = DISABLE;
	hcan1.Init.ReceiveFifoLocked = DISABLE;
	hcan1.Init.TimeTriggeredMode = DISABLE;
	hcan1.Init.TransmitFifoPriority = DISABLE;

	//Settings related to the CAN bit timings
	hcan1.Init.Prescaler = 5; // 25 MHz / 5 = 5 MHz CAN-Takt
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan1.Init.TimeSeg1 = CAN_BS1_8TQ;
	hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
	/*---------------------------------------------------------*/
	// Total-TQ = 1 (Sync) + 8 (BS1) + 1 (BS2) = 10 TQ
	// Sample Point = (1 + 8) / 10 = 90% (perfect standard value!)
	//Baudrate = 5 MHz / 10 TQ = 500 Kbit/s

	//Initialization of CAN1 peripheral
	if(HAL_CAN_Init(&hcan1) != HAL_OK)
		Error_Handler();

}

void CAN1_TX(void)
{

	CAN_TxHeaderTypeDef TxHeader;

	uint32_t TxMailbox;

	uint8_t message;

	//CAN header configuration
	TxHeader.DLC = 1; //sending 1byte message
	TxHeader.StdId = 0x65A;
	TxHeader.IDE = CAN_ID_STD; //Standard ID
	TxHeader.RTR = CAN_RTR_DATA; //Data frame

	message = ++led_no;

	if(led_no == 4)
	{
		led_no = 0;
	}

	//Add message to the first free Tx mailbox and set the transmission request bit (TXRQ = 1).
	if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &message, &TxMailbox) != HAL_OK)
	{
		//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); //for observation purpose
		Error_Handler();
	}

	//HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12); //just for observation purpose

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
	htimer6.Init.Prescaler = 20000 -1; // 50MHz / 20.000 = 2.500Hz => 1 tick every 0.4 msec.
	htimer6.Init.Period = 2500 - 1;   // 2500 ticks/sec.

	if(HAL_TIM_Base_Init(&htimer6) != HAL_OK)
	{
		Error_Handler();
	}
}

void GPIO_Init(void)
{
	GPIO_InitTypeDef led_gpio = {0};

	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//LED Pins configuration
	led_gpio.Mode = GPIO_MODE_OUTPUT_PP;
	led_gpio.Pull = GPIO_NOPULL;
	led_gpio.Speed = GPIO_SPEED_FREQ_LOW;
	led_gpio.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOD, &led_gpio);

	//Start button configuration
	led_gpio.Mode = GPIO_MODE_IT_FALLING; //interrupt generation by pressing the button
	led_gpio.Pull = GPIO_NOPULL;
	led_gpio.Speed = GPIO_SPEED_FREQ_LOW;
	led_gpio.Pin = GPIO_PIN_0;
	HAL_GPIO_Init(GPIOA, &led_gpio);

	//Enable the IRQ for EXTI0
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef RxHeader;
	char  msg[50];
	uint8_t rcvd_msg[8];

	//Now get the CAN frame from RX_FIFO0
	if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, rcvd_msg) != HAL_OK)
		Error_Handler();

	if(RxHeader.StdId == 0x65D && RxHeader.RTR == 0)
	{
		//Data frame sent by N1 to N2

		LED_Manage_Output(rcvd_msg[0]);

		sprintf(msg, "N1 message received: %u\r\n", rcvd_msg[0]);
	}
	else if(RxHeader.StdId == 0x65A && RxHeader.RTR == 1)
	{
		//Remote frame sent by N1 to N2 (request)

		send_response(RxHeader.StdId);

		return;
	}
	else if(RxHeader.StdId == 0x65A && RxHeader.RTR == 0)
	{
		//Data frame sent by N2 to N1 (reply)

		sprintf(msg, "N2 reply received: %X\r\n", rcvd_msg[0] << 8 | rcvd_msg[1]);
	}

	HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

void send_response(uint32_t Id)
{

	CAN_TxHeaderTypeDef TxHeader;

	uint32_t TxMailbox;

	uint8_t message[2] = {0xAB, 0xCD};

	//CAN header configuration
	TxHeader.DLC = 2; //sending 2 byte message
	TxHeader.StdId = Id;
	TxHeader.IDE = CAN_ID_STD; //Standard ID
	TxHeader.RTR = CAN_RTR_DATA; //Data frame

	//Add message to the first free Tx mailbox and set the transmission request bit (TXRQ = 1).
	if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, message, &TxMailbox) != HAL_OK)
	{
		//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); //for observation purpose
		Error_Handler();
	}

	//HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12); //just for observation purpose

}

void LED_Manage_Output(uint8_t led_number)
{
	switch (led_number) {
		case 1:
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
			break;

		case 2:
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
			break;

		case 3:
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
			break;

		case 4:
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
			break;

		default:
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
			break;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	CAN_TxHeaderTypeDef TxHeader;

	uint32_t TxMailbox;
	char msg[50];
	uint8_t message = 0;
	if(htim->Instance == TIM6)
	{
		if(req_counter == 4)
		{
			//N1 sending remote frame to N2 after 4 sec.

			TxHeader.DLC = 2; //sending 2 byte message
			TxHeader.StdId = 0x65A;
			TxHeader.IDE = CAN_ID_STD; //Standard ID
			TxHeader.RTR = CAN_RTR_REMOTE; //Data frame

			//Add message to the first free Tx mailbox and set the transmission request bit (TXRQ = 1).
			if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, &message, &TxMailbox) != HAL_OK)
			{
				//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); //for observation purpose
				Error_Handler();
			}

			sprintf(msg, "N1 remote frame sent \r\n");
			HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

			req_counter = 0;
		}
		else
		{
			CAN1_TX();
			req_counter++;
		}
		//HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13); //for observation purpose
	}


}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef * hcan)
{
	//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); //for observation purpose
}

void Error_Handler(void)
{
	while(1);
}
