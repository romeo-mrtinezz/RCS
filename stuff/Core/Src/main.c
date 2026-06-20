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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bmi088.h"
#include "stm32g483xx.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_spi.h"
#include <stdint.h>
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

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;

PCD_HandleTypeDef hpcd_USB_FS;

/* USER CODE BEGIN PV */
uint8_t byte_2;

uint8_t rate_x_lsb;
uint8_t rate_x_msb;
int16_t rate_x_raw;

uint8_t rate_y_lsb;
uint8_t rate_y_msb;
int16_t rate_y_raw;

uint8_t rate_z_lsb;
uint8_t rate_z_msb;
int16_t rate_z_raw;

float rate_x;
float rate_y;
float rate_z;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM1_Init(void);
static void MX_USB_PCD_Init(void);

/* USER CODE BEGIN PFP */
void run_blinky() {
  // Use function in while loop
  HAL_GPIO_WritePin(GPIOA, RED_LED_Pin, GPIO_PIN_SET);
  HAL_Delay(1000);
  HAL_GPIO_WritePin(GPIOA, RED_LED_Pin, GPIO_PIN_RESET);
  HAL_Delay(1000);
};

void gyro_spi_read(uint8_t address) {
  // Calculate byte 1 from address and R command
  uint8_t byte_1 = address | 0x80;

  // 1. Send address and R command
  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_RESET); // Chip select low
  // HAL_Delay(100);
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT); // Send byte 1
  // HAL_Delay(100);

  // 2. Read data
  HAL_SPI_Receive(&hspi1, &byte_2, BYTE_SIZE, TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_SET);  // End comms
  // don't have to return anything because we passed in address of byte 2
};


uint8_t initialise_accel() {
  uint8_t byte_1 = ACC_PWR_CTRL | 0x00; // Write mode
  uint8_t pwr_set = 0x04;
  uint8_t accel_id;
  uint8_t dummy;

  // 1. Switch to spi mode
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_SET); // rising edge to switch to spi
  // HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_SET); // deselect gyro


  // 2. Switch from suspend mode to normal power mode
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT); // Send byte 1
  HAL_SPI_Transmit(&hspi1, &pwr_set, BYTE_SIZE, TIMEOUT); // Send byte 1
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_SET); 
  HAL_Delay(1);

  // 3. Read accel id
  byte_1 = ACC_CHIP_ID | 0x80;
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_RESET); // pull cs low
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT); // Send byte 1
  HAL_SPI_Receive(&hspi1, &dummy, BYTE_SIZE, TIMEOUT); // dummy
  HAL_SPI_Receive(&hspi1, &accel_id, BYTE_SIZE, TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_SET); // end transaction
  
  return accel_id; // expect 0x1E = 30 
};

uint8_t accel_spi_read(uint8_t address) {
  uint8_t result;
  uint8_t dummy;

  uint8_t byte_1 = address | 0x80; // R mode
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_RESET); // pull cs low
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT); // Send byte 1
  HAL_SPI_Receive(&hspi1, &dummy, BYTE_SIZE, TIMEOUT); // dummy
  HAL_SPI_Receive(&hspi1, &result, BYTE_SIZE, TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_SET); // end transaction
  
  return result; 
};

/** 
  * @brief read all 3 axis at once
  * @param first_address
  * @retval None
*/
AccData accel_burst_read(uint8_t first_address) {
  uint8_t dummy;
  uint8_t byte_1 = first_address | 0x80; // R mode
  uint8_t acc_buffer[6]; // 6 bytes of data
  AccData accel_data;

  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_RESET); // pull cs low
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT); // Send byte 1
  HAL_SPI_Receive(&hspi1, &dummy, BYTE_SIZE, TIMEOUT); // dummy
  HAL_SPI_Receive(&hspi1, acc_buffer, sizeof(acc_buffer), TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_SET); // end transaction

  // cast to int due to 2's complement
  // default acc_range 0x01, +-6g
  // Accel in mg
  accel_data.acc_x = (int16_t)(acc_buffer[1] << 8 | acc_buffer[0])/32768.0f * 1000.0f * 4.0f * 1.5f; // msb*256+lsb 
  accel_data.acc_y = (int16_t)(acc_buffer[3] << 8 | acc_buffer[2])/32768.0f * 1000.0f * 4.0f * 1.5f;
  accel_data.acc_z = (int16_t)(acc_buffer[5] << 8 | acc_buffer[4])/32768.0f * 1000.0f * 4.0f * 1.5f;

  return accel_data;
};

GyroData gyro_burst_read(uint8_t first_address) {
  uint8_t byte_1 = first_address | 0x80; // R mode
  uint8_t gyro_buffer[6]; // 6 bytes of data
  GyroData gyro_data;

  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT);
  HAL_SPI_Receive(&hspi1, gyro_buffer, sizeof(gyro_buffer), TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_SET);

  gyro_data.rate_x = (int16_t)(gyro_buffer[1] << 8 | gyro_buffer[0]) * 0.061f;
  gyro_data.rate_y = (int16_t)(gyro_buffer[3] << 8 | gyro_buffer[2]) * 0.061f;
  gyro_data.rate_z = (int16_t)(gyro_buffer[5] << 8 | gyro_buffer[4]) * 0.061f;

  return gyro_data;
};

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_USB_PCD_Init();
  /* USER CODE BEGIN 2 */
  // HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  // TIM1->CCR1 = 5000; // 50% duty cycle for ARR = 10,000

  uint8_t acc_x_lsb;
  uint8_t acc_x_msb; 
  int16_t acc_x_raw;

  uint8_t acc_y_lsb;
  uint8_t acc_y_msb; 
  int16_t acc_y_raw;

  uint8_t acc_z_lsb;
  uint8_t acc_z_msb;
  int16_t acc_z_raw;

  float acc_x;
  float acc_y;
  float acc_z;

  byte_2 = initialise_accel();
  AccData accel_data;
  GyroData gyro_data;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  { 
    accel_data = accel_burst_read(ACC_X_LSB);
    gyro_data = gyro_burst_read(ADDR_RATE_X_LSB);
    // gyro_spi_read(GYRO_CHIP_ID);

    // gyro_spi_read(ADDR_RATE_X_LSB);
    // rate_x_lsb = byte_2;
    // gyro_spi_read(ADDR_RATE_X_MSB);
    // rate_x_msb = byte_2;
    // gyro_spi_read(ADDR_RATE_Y_LSB);
    // rate_y_lsb = byte_2;
    // gyro_spi_read(ADDR_RATE_Y_MSB);
    // rate_y_msb = byte_2;
    // gyro_spi_read(ADDR_RATE_Z_LSB);
    // rate_z_lsb = byte_2;
    // gyro_spi_read(ADDR_RATE_Z_MSB);
    // rate_z_msb = byte_2;

    // rate_x_raw = (rate_x_msb << 8 | rate_x_lsb); // msb*256+lsb
    // rate_y_raw = (rate_y_msb << 8 | rate_y_lsb); 
    // rate_z_raw = (rate_z_msb << 8 | rate_z_lsb); 
    // rate_x = rate_x_raw * 0.061f; // degrees/s
    // rate_y = rate_y_raw * 0.061f; 
    // rate_z = rate_z_raw * 0.061f;     

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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  hspi2.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 160-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 9999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USB Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.Sof_enable = DISABLE;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RED_LED_Pin|BLUE_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin|CS_ACCEL_Pin|CS_SD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PG10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : RED_LED_Pin BLUE_LED_Pin */
  GPIO_InitStruct.Pin = RED_LED_Pin|BLUE_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_GYRO_Pin CS_ACCEL_Pin CS_SD_Pin */
  GPIO_InitStruct.Pin = CS_GYRO_Pin|CS_ACCEL_Pin|CS_SD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : GYRO_INTR_Pin ACCEL_INTR_Pin */
  GPIO_InitStruct.Pin = GYRO_INTR_Pin|ACCEL_INTR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
