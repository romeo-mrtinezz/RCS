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
#include "cmsis_os2.h"
#include "stm32g4xx_hal.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pid.h"
#include "bmi088.h"
#include "sd.h"
#include "global.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern AccData accel_data;
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
/* Definitions for log */
osThreadId_t logHandle;
const osThreadAttr_t log_attributes = {
  .name = "log",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for messageQueue */
osMessageQueueId_t messageQueueHandle;
const osMessageQueueAttr_t messageQueue_attributes = {
  .name = "messageQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartReadIMU(void *argument);
void StartPidUpdate(void *argument);
void StartSelectThruster(void *argument);
void StartLog(void *argument);

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

  /* Create the queue(s) */
  /* creation of messageQueue */
  messageQueueHandle = osMessageQueueNew (12, sizeof(MessageQueue_t), &messageQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of readIMU */
  readIMUHandle = osThreadNew(StartReadIMU, NULL, &readIMU_attributes);

  /* creation of pidUpdate */
  pidUpdateHandle = osThreadNew(StartPidUpdate, NULL, &pidUpdate_attributes);

  /* creation of selectThruster */
  selectThrusterHandle = osThreadNew(StartSelectThruster, NULL, &selectThruster_attributes);

  /* creation of log */
  logHandle = osThreadNew(StartLog, NULL, &log_attributes);

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
    MessageQueue_t msg;
  /* Infinite loop */
  for(;;)
  {
    accel_burst_read(&accel_data);
    msg.timestamp = xTaskGetTickCount(); // uint32? 
    msg.acc_x = accel_data.acc_x;
    msg.acc_y = accel_data.acc_y;
    msg.acc_z = accel_data.acc_z;
    osMessageQueuePut(messageQueueHandle, &msg, 0, 0);
    osDelay(100); // 10Hz
  }
  /* USER CODE END StartReadIMU */
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

/* USER CODE BEGIN Header_StartLog */
/**
* @brief Function implementing the log thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLog */
void StartLog(void *argument)
{
  /* USER CODE BEGIN StartLog */
  MessageQueue_t buffer[12];
  SD_Card_init();
  /* Infinite loop */
  for(;;)
  {
    // 0 timeout means will return immediately - I want this because I want to maintain the 1Hz
    // There are 12 elements in buffer, so there should always be 10 in there anyways since I'm reading at 10Hz
    // osMessageQueueGet(messageQueueHandle, NULL, 0, 0); // this only grabs 1 element, not all 10
    uint8_t count = 0;
    while (count < 10 && osMessageQueueGet(messageQueueHandle, &buffer[count], NULL, 0) == osOK) {
        count++;
    }

    log_accel(count, buffer, osThreadGetId());
    osDelay(1000); // 1Hz, every second
  }
  /* USER CODE END StartLog */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

