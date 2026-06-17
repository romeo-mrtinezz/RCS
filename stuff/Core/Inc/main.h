/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PWM1_Pin GPIO_PIN_0
#define PWM1_GPIO_Port GPIOC
#define PWM2_Pin GPIO_PIN_1
#define PWM2_GPIO_Port GPIOC
#define PWM3_Pin GPIO_PIN_2
#define PWM3_GPIO_Port GPIOC
#define PWM4_Pin GPIO_PIN_3
#define PWM4_GPIO_Port GPIOC
#define RED_LED_Pin GPIO_PIN_0
#define RED_LED_GPIO_Port GPIOA
#define BLUE_LED_Pin GPIO_PIN_1
#define BLUE_LED_GPIO_Port GPIOA
#define CS_GYRO_Pin GPIO_PIN_4
#define CS_GYRO_GPIO_Port GPIOC
#define CS_ACCEL_Pin GPIO_PIN_5
#define CS_ACCEL_GPIO_Port GPIOC
#define GYRO_INTR_Pin GPIO_PIN_0
#define GYRO_INTR_GPIO_Port GPIOB
#define ACCEL_INTR_Pin GPIO_PIN_1
#define ACCEL_INTR_GPIO_Port GPIOB
#define CS_SD_Pin GPIO_PIN_6
#define CS_SD_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
