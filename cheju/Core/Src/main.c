/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
uint32_t now = HAL_GetTick();
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* 发两路雷达数据给TC264，帧头区分：0xBB=USART3那路，0xCC=USART2那路 */

void Radar_Send_Frame(void)
{
    static uint16_t last_cnt3 = 0;
    static uint16_t last_cnt2 = 0;
    uint8_t frame[6];
    uint8_t sent = 0;

    if (radar_ch3.receive_cnt != last_cnt3)
    {
        last_cnt3 = radar_ch3.receive_cnt;
        frame[0] = 0xBB;
        frame[1] = (uint8_t)(radar_ch3.distance & 0xFF);
        frame[2] = (uint8_t)(radar_ch3.distance >> 8);
        frame[3] = radar_ch3.confidence;
        frame[4] = frame[1] ^ frame[2] ^ frame[3];
        frame[5] = 0x0D;
        HAL_UART_Transmit(&huart1, frame, 6, 10);
        sent = 1;
    }

    if (radar_ch2.receive_cnt != last_cnt2)
    {
        last_cnt2 = radar_ch2.receive_cnt;
        frame[0] = 0xCC;
        frame[1] = (uint8_t)(radar_ch2.distance & 0xFF);
        frame[2] = (uint8_t)(radar_ch2.distance >> 8);
        frame[3] = radar_ch2.confidence;
        frame[4] = frame[1] ^ frame[2] ^ frame[3];
        frame[5] = 0x0D;
        HAL_UART_Transmit(&huart1, frame, 6, 10);
        sent = 1;
    }

    //if (sent) HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
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
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	/* 确认程序跑到这里了 */
//for (uint8_t i = 0; i < 9; i++)
//{
//    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//    HAL_Delay(150);
//}
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

//huart1.RxState = HAL_UART_STATE_READY;
//huart1.gState  = HAL_UART_STATE_READY;
//__HAL_UNLOCK(&huart1);
  HAL_UART_Receive_IT(&huart1, Usart1_Receive_buf, 1);
	
  HAL_UART_Receive_IT(&huart2, Uart2_Receive_buf, 1);
  HAL_UART_Receive_IT(&huart3, Uart3_Receive_buf, 1);
//if (HAL_UART_Receive_IT(&huart1, Usart1_Receive_buf, 1) != HAL_OK)
//{
//    // 启动失败,让PC13长亮提示
//    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
//    while(1);
//}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE BEGIN WHILE */

// while(1) 里:


if (now >= 10000 && now < 13000)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);   // 10~13秒:转
}
else
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); // 其它时间:停
}
    Radar_Send_Frame();   // 发帧 + LED发送指示
    HAL_Delay(30);        // 约10Hz；想反应更快就减小这个值（比如20）

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
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

  /** Initializes the CPU, AHB and APB buses clocks
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

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
