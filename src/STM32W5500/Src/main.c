/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* vim: set ai et ts=4 sw=4: */
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include "socket.h"
#include "dhcp.h"
#include "dns.h"
#include "mqtt_interface.h"
#include "MQTTClient.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void msTick_Handler(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


#define DHCP_SOCKET     0
#define DNS_SOCKET      1
#define MQQTT_SOCKET     2

// Receive Buffer
#define BUFFER_SIZE	2048
uint8_t tempBuffer[BUFFER_SIZE];
// Global variables
uint32_t dhcp_counter;
uint8_t mqtt_push_counter;
uint8_t mqtt_flag;
uint16_t mes_id;
wiz_NetInfo net_info2;

int8_t str_printf(char *StrBuff, uint8_t BuffLen, const char *args, ...);

void messageArrived(MessageData* md);

void UART_Printf(const char* fmt, ...)
{
	char buff[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buff, sizeof(buff), fmt, args);
	HAL_UART_Transmit(&huart1,
		(uint8_t*) buff,
		strlen(buff),
		HAL_MAX_DELAY);
	va_end(args);
}

void W5500_Select(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}

void W5500_Unselect(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

void W5500_ReadBuff(uint8_t* buff, uint16_t len)
{
	HAL_SPI_Receive(&hspi2, buff, len, HAL_MAX_DELAY);
}

void W5500_WriteBuff(uint8_t* buff, uint16_t len)
{
	HAL_SPI_Transmit(&hspi2, buff, len, HAL_MAX_DELAY);
}

uint8_t W5500_ReadByte(void)
{
	uint8_t byte;
	W5500_ReadBuff(&byte, sizeof(byte));
	return byte;
}

void W5500_WriteByte(uint8_t byte)
{
	W5500_WriteBuff(&byte, sizeof(byte));
}

volatile bool ip_assigned = false;

void Callback_IPAssigned(void)
{
	UART_Printf("Callback: IP assigned! Leased time: %d sec\r\n",
		getDHCPLeasetime());
	ip_assigned = true;
}

void Callback_IPConflict(void)
{
	UART_Printf("Callback: IP conflict!\r\n");
}

// 1K should be enough, see https://forum.wiznet.io/t/topic/1612/2
uint8_t dhcp_buffer[1024];
// 1K seems to be enough for this buffer as well
uint8_t dns_buffer[1024];

void init(void);

void loop()
{
	HAL_Delay(1000);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	init();
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		Network n;
		MQTTClient c;
		n.my_socket = 0;
		uint8_t buf[100];
		int32_t rc = 0;

		uint8_t targetIP[4] =
		{
			192,
			168,
			29,
			42
		};       
	//DNS_init(1, tempBuffer);
	//DNS_run(gWIZNETINFO.dns, "test.mosquitto.org", targetIP);

	NewNetwork(&n, MQQTT_SOCKET);
		ConnectNetwork(&n, targetIP, 1883);
		MQTTClientInit(&c, &n, 1000, buf, 100, tempBuffer, BUFFER_SIZE);

		MQTTPacket_connectData data = MQTTPacket_connectData_initializer
		;
		data.willFlag = 0;
		data.MQTTVersion = 4; 	  //3;
		data.clientID.cstring = (char*) "w5500-client";
		data.username.cstring = "username";
		data.password.cstring = "";
		data.keepAliveInterval = 60;
		data.cleansession = 1;
		rc = MQTTConnect(&c, &data);
		UART_Printf("Connected %d\r\n", rc);

		char SubString[] = "/#";   
		rc = MQTTSubscribe(&c, SubString, QOS0, messageArrived);
		UART_Printf("Subscribed (%s) %d\r\n", SubString, rc);
		
		uint32_t count = 0;

		while (1)
		{
			if(mqtt_flag)
			{
				mqtt_flag = 0;

				char message[16];
				int8_t len = str_printf(message, sizeof(message), "%d.%d.%d.%d", net_info2.ip[0], net_info2.ip[1], net_info2.ip[2], net_info2.ip[3]);
				if (len > 0)
				{
					MQTTMessage pubMessage;
					pubMessage.qos = QOS0;
					pubMessage.id = mes_id++;
					pubMessage.payloadlen = len;
					pubMessage.payload = message;
					MQTTPublish(&c, "/w5500_stm32_client", &pubMessage);


					count++;
					if (count >= 10)
					{
						HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
						count = 0;
					}
				}

			}

			// ???? ??????? ? ??????????? ????????? ? ????????
			MQTTYield(&c, 1000);
			msTick_Handler();					
		}

		loop();

	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
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
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 
                           PA4 PA5 PA6 PA7 
                           PA8 PA11 PA12 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 
                          |GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10 
                           PB11 PB3 PB4 PB5 
                           PB6 PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10 
                          |GPIO_PIN_11|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

//==============================================================================
// ?????????? ????????? ?? ??????? (?????? = 1 ??)
//==============================================================================
void msTick_Handler(void)
{
	MilliTimer_Handler();

	if (++dhcp_counter >= 500)  // 1000 - ??????? //25???????? ????????
	
		{
			dhcp_counter = 0;
			DHCP_time_handler();  // ??????? ??? ????????? ????-????? DHCP

			if(++mqtt_push_counter >= 10)  // ?????? 10 ??????
			{
				mqtt_push_counter = 0;
				mqtt_flag = 1;
			}
		}
}

//==============================================================================
// ?????????? ????????? MQTT, ??????????????? ????????? ???????
//==============================================================================
void messageArrived(MessageData* md)
{
	MQTTMessage* message = md->message;

	for (uint8_t i = 0; i < md->topicName->lenstring.len; i++)
		putchar(*(md->topicName->lenstring.data + i));

	UART_Printf(" (%.*s)\r\n",
		(int32_t) message->payloadlen,
		(char*) message->payload);
}

//==============================================================================
//==============================================================================
// ??????? ?????????? ??????????????? ?????? ? ????? StrBuff
//==============================================================================
int8_t str_printf(char *StrBuff, uint8_t BuffLen, const char *args, ...)
{
	va_list ap;
	va_start(ap, args);
	int8_t len = vsnprintf(StrBuff, BuffLen, args, ap);
	va_end(ap);
	return len;
}
//==============================================================================

void init(void)
{
	UART_Printf("\r\ninit() called!\r\n");

	UART_Printf("Registering W5500 callbacks...\r\n");
	reg_wizchip_cs_cbfunc(W5500_Select, W5500_Unselect);
	reg_wizchip_spi_cbfunc(W5500_ReadByte, W5500_WriteByte);
	reg_wizchip_spiburst_cbfunc(W5500_ReadBuff, W5500_WriteBuff);

	UART_Printf("Calling wizchip_init()...\r\n");
	uint8_t rx_tx_buff_sizes[] =
	{
		2,
		2,
		2,
		2,
		2,
		2,
		2,
		2
	};
	wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes);

	UART_Printf("Calling DHCP_init()...\r\n");
	wiz_NetInfo net_info =
	{
		.mac = {
		0xEA,
		0x11,
		0x22,
		0x33,
		0x44,
		0xEA
	},
		.dhcp = NETINFO_DHCP
	};
	// set MAC address before using DHCP
	setSHAR(net_info.mac);
	DHCP_init(DHCP_SOCKET, dhcp_buffer);

	UART_Printf("Registering DHCP callbacks...\r\n");
	reg_dhcp_cbfunc(Callback_IPAssigned,
		Callback_IPAssigned,
		Callback_IPConflict);

	UART_Printf("Calling DHCP_run()...\r\n");
	// actually should be called in a loop, e.g. by timer
	uint32_t ctr = 100000;
	while ((!ip_assigned) && (ctr > 0))
	{
		DHCP_run();
		ctr--;
	}
	if (!ip_assigned)
	{
		UART_Printf("\r\nIP was not assigned :(\r\n");
		return;
	}

	getIPfromDHCP(net_info.ip);
	getGWfromDHCP(net_info.gw);
	getSNfromDHCP(net_info.sn);

	uint8_t dns[4];
	getDNSfromDHCP(dns);

	UART_Printf(
		"IP:  %d.%d.%d.%d\r\nGW:  %d.%d.%d.%d\r\nNet: %d.%d.%d.%d\r\nDNS: %d.%d.%d.%d\r\n",
		net_info.ip[0],
		net_info.ip[1],
		net_info.ip[2],
		net_info.ip[3],
		net_info.gw[0],
		net_info.gw[1],
		net_info.gw[2],
		net_info.gw[3],
		net_info.sn[0],
		net_info.sn[1],
		net_info.sn[2],
		net_info.sn[3],
		dns[0],
		dns[1],
		dns[2],
		dns[3]);

	UART_Printf("Calling wizchip_setnetinfo()...\r\n");
	wizchip_setnetinfo(&net_info);

	UART_Printf("Calling DNS_init()...\r\n");
	DNS_init(DNS_SOCKET, dns_buffer);

	/*    uint8_t addr[4];
	 {
	 char domain_name[] = "eax.me";
	 UART_Printf("Resolving domain name \"%s\"...\r\n", domain_name);
	 int8_t res = DNS_run(dns, (uint8_t*) &domain_name, addr);
	 if (res != 1)
	 {
	 UART_Printf("DNS_run() failed, res = %d", res);
	 return;
	 }
	 UART_Printf("Result: %d.%d.%d.%d\r\n", addr[0], addr[1], addr[2],
	 addr[3]);
	 }

	 UART_Printf("Creating socket...\r\n");
	 uint8_t http_socket = HTTP_SOCKET;
	 uint8_t code = socket(http_socket, Sn_MR_TCP, 10888, 0);
	 if (code != http_socket)
	 {
	 UART_Printf("socket() failed, code = %d\r\n", code);
	 return;
	 }

	 UART_Printf("Socket created, connecting...\r\n");
	 code = connect(http_socket, addr, 80);
	 if (code != SOCK_OK)
	 {
	 UART_Printf("connect() failed, code = %d\r\n", code);
	 close(http_socket);
	 return;
	 }

	 UART_Printf("Connected, sending HTTP request...\r\n");
	 {
	 char req[] = "GET / HTTP/1.0\r\nHost: eax.me\r\n\r\n";
	 uint16_t len = sizeof(req) - 1;
	 uint8_t* buff = (uint8_t*) &req;
	 while (len > 0)
	 {
	 UART_Printf("Sending %d bytes...\r\n", len);
	 int32_t nbytes = send(http_socket, buff, len);
	 if (nbytes <= 0)
	 {
	 UART_Printf("send() failed, %d returned\r\n", nbytes);
	 close(http_socket);
	 return;
	 }
	 UART_Printf("%d bytes sent!\r\n", nbytes);
	 len -= nbytes;
	 }
	 }

	 UART_Printf("Request sent. Reading response...\r\n");
	 {
	 char buff[32];
	 for (;;)
	 {
	 int32_t nbytes = recv(http_socket, (uint8_t*) &buff,
	 sizeof(buff) - 1);
	 if (nbytes == SOCKERR_SOCKSTATUS)
	 {
	 UART_Printf("\r\nConnection closed.\r\n");
	 break;
	 }

	 if (nbytes <= 0)
	 {
	 UART_Printf("\r\nrecv() failed, %d returned\r\n", nbytes);
	 break;
	 }

	 buff[nbytes] = '\0';
	 UART_Printf("%s", buff);
	 }
	 }*

	 UART_Printf("Closing socket.\r\n");
	 close(http_socket);*/
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
	  /* User can add his own implementation to report the file name and line number,
	   tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
