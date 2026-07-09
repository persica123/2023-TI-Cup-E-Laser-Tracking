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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "motor.h"
#include "../../Hardware/encoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "control.h"
#include "key.h"
#include "jy61p.h"

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

/* USER CODE BEGIN PV */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint32_t last_control_tick = 0;
static uint32_t last_oled_tick = 0;
static char last_oled_lines[4][17] = {0};

static void Format_Float_1(char *line, size_t line_size, const char *name, float value)
{
    int32_t value10 = (int32_t)(value * 10.0f);
    char sign = ' ';

    if (value10 < 0) {
        sign = '-';
        value10 = -value10;
    }

    snprintf(line, line_size, "%s:%c%3ld.%01ld",
             name,
             sign,
             (long)(value10 / 10),
             (long)(value10 % 10));
}

static void OLED_Show_Line_Changed(uint8_t line_index, uint8_t y, const char *text)
{
    char padded[17];

    memset(padded, ' ', 16);
    padded[16] = '\0';
    for (uint8_t i = 0; i < 16 && text[i] != '\0'; i++) {
        padded[i] = text[i];
    }

    if (memcmp(last_oled_lines[line_index], padded, 16) != 0) {
        OLED_ShowLine16(y, padded, &hi2c1);
        memcpy(last_oled_lines[line_index], padded, 17);
    }
}

static void OLED_Show_Gyroscope(void)
{
    char line[17];

    Format_Float_1(line, sizeof(line), "x", fAngle[0]);
    OLED_Show_Line_Changed(0, 0, line);

    Format_Float_1(line, sizeof(line), "y", fAngle[1]);
    OLED_Show_Line_Changed(1, 2, line);

    Format_Float_1(line, sizeof(line), "z", fAngle[2]);
    OLED_Show_Line_Changed(2, 4, line);

    Format_Float_1(line, sizeof(line), "yaw", fYaw);
    OLED_Show_Line_Changed(3, 6, line);
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
  MX_I2C1_Init();
  MX_TIM9_Init();
  MX_TIM5_Init();
  MX_TIM6_Init();
  MX_TIM8_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */
	OLED_Init(&hi2c1);
    OLED_Clear(0, &hi2c1);
    memset(last_oled_lines, 0, sizeof(last_oled_lines));
    OLED_ShowString(0, 0, (uint8_t *)"Car Ready", 16, &hi2c1);

    Motor_Init();
    Encoder_Init(&Encoder_L, &htim5);
    Encoder_Init(&Encoder_R, &htim8);
    Control_Init();         // ???????
    Key_Init();             // ?????
    HAL_Delay(200);

    gyroscope_Init();

    last_control_tick = HAL_GetTick();
    last_oled_tick = HAL_GetTick();

    // ????????,????????
    Control_SetMode(MODE_IDLE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
     uint32_t now = HAL_GetTick();
    // ???? 10ms (100Hz)
    if ((now - last_control_tick) >= 10U)
    {
        last_control_tick += 10U;
        if ((now - last_control_tick) >= 10U)
        {
            last_control_tick = now;
        }
        Encoder_Update(&Encoder_L);
        Encoder_Update(&Encoder_R);
        Control_Task();     // ??????
    }

    gyroscope_scan();

    // OLED ???? 50ms (20Hz)
    if ((now - last_oled_tick) >= 50U)
    {
        last_oled_tick = now;
        OLED_Show_Gyroscope();
    }

    // ????
    Key_Scan();
			
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
