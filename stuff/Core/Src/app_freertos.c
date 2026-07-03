/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for readIMU */
osThreadId_t readIMUHandle;
const osThreadAttr_t readIMU_attributes = {
  .name = "readIMU",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for compFilter */
osThreadId_t compFilterHandle;
const osThreadAttr_t compFilter_attributes = {
  .name = "compFilter",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for pidUpdate */
osThreadId_t pidUpdateHandle;
const osThreadAttr_t pidUpdate_attributes = {
  .name = "pidUpdate",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for selectThruster */
osThreadId_t selectThrusterHandle;
const osThreadAttr_t selectThruster_attributes = {
  .name = "selectThruster",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartReadIMU(void *argument);
void StartCompFilter(void *argument);
void StartPidUpdate(void *argument);
void StartSelectThruster(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of readIMU */
  readIMUHandle = osThreadNew(StartReadIMU, NULL, &readIMU_attributes);

  /* creation of compFilter */
  compFilterHandle = osThreadNew(StartCompFilter, NULL, &compFilter_attributes);

  /* creation of pidUpdate */
  pidUpdateHandle = osThreadNew(StartPidUpdate, NULL, &pidUpdate_attributes);

  /* creation of selectThruster */
  selectThrusterHandle = osThreadNew(StartSelectThruster, NULL, &selectThruster_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartReadIMU */
/**
  * @brief  Function implementing the readIMU thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartReadIMU */
void StartReadIMU(void *argument)
{
  /* USER CODE BEGIN StartReadIMU */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartReadIMU */
}

/* USER CODE BEGIN Header_StartCompFilter */
/**
* @brief Function implementing the compFilter thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCompFilter */
void StartCompFilter(void *argument)
{
  /* USER CODE BEGIN StartCompFilter */
  /* Infinite loop */
  for(;;)
  {
    // float angle = alpha*(prev_angle + gyro_rate * dt) + (1-alpha)*accel_angle;
    osDelay(1);
  }
  /* USER CODE END StartCompFilter */
}

/* USER CODE BEGIN Header_StartPidUpdate */
/**
* @brief Function implementing the pidUpdate thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartPidUpdate */
void StartPidUpdate(void *argument)
{
  /* USER CODE BEGIN StartPidUpdate */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartPidUpdate */
}

/* USER CODE BEGIN Header_StartSelectThruster */
/**
* @brief Function implementing the selectThruster thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSelectThruster */
void StartSelectThruster(void *argument)
{
  /* USER CODE BEGIN StartSelectThruster */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartSelectThruster */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

