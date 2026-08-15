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
#include "ff.h"
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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define NRST_Pin GPIO_PIN_10
#define NRST_GPIO_Port GPIOG
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
#define SAFE_Pin GPIO_PIN_2
#define SAFE_GPIO_Port GPIOA
#define PT1_IN_Pin GPIO_PIN_4
#define PT1_IN_GPIO_Port GPIOA
#define CS_GYRO_Pin GPIO_PIN_4
#define CS_GYRO_GPIO_Port GPIOC
#define CS_ACCEL_Pin GPIO_PIN_5
#define CS_ACCEL_GPIO_Port GPIOC
#define GYRO_INTR_Pin GPIO_PIN_0
#define GYRO_INTR_GPIO_Port GPIOB
#define ACCEL_INTR_Pin GPIO_PIN_1
#define ACCEL_INTR_GPIO_Port GPIOB
#define PT2_IN_Pin GPIO_PIN_2
#define PT2_IN_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_10
#define BUZZER_GPIO_Port GPIOB
#define CS_SD_Pin GPIO_PIN_6
#define CS_SD_GPIO_Port GPIOC
#define EXTRA_TX_Pin GPIO_PIN_9
#define EXTRA_TX_GPIO_Port GPIOA
#define EXTRA_RX_Pin GPIO_PIN_10
#define EXTRA_RX_GPIO_Port GPIOA
#define RFD_RTS_Pin GPIO_PIN_15
#define RFD_RTS_GPIO_Port GPIOA
#define RFD_TX_Pin GPIO_PIN_10
#define RFD_TX_GPIO_Port GPIOC
#define RFD_RX_Pin GPIO_PIN_11
#define RFD_RX_GPIO_Port GPIOC
#define RS422_TX_Pin GPIO_PIN_12
#define RS422_TX_GPIO_Port GPIOC
#define RS422_RX_Pin GPIO_PIN_2
#define RS422_RX_GPIO_Port GPIOD
#define RFD_CTS_Pin GPIO_PIN_7
#define RFD_CTS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
