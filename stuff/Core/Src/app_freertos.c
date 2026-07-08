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
#include "pid.h"
#include "bmi088.h"
#include "sd.h"
#include "global.h"
#include <stdint.h>
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
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 128 * 4
};
/* Definitions for log */
osThreadId_t logHandle;
const osThreadAttr_t log_attributes = {
  .name = "log",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 551 * 4
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
void StartLog(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
   while(1) {
    // stay here don't corrupt memory
   }
}
/* USER CODE END 4 */

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
    uint32_t hlw;
  /* Infinite loop */
  for(;;)
  {
    accel_burst_read(&accel_data);
    msg.timestamp = xTaskGetTickCount(); // uint32? 
    msg.acc_x = accel_data.acc_x;
    msg.acc_y = accel_data.acc_y;
    msg.acc_z = accel_data.acc_z;
    osMessageQueuePut(messageQueueHandle, &msg, 0, 0);
    // HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port,  BLUE_LED_Pin);
    // osDelay(1);
    // HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port,  BLUE_LED_Pin);
    // osDelay(1);    
    osDelay(100); // 10Hz
    hlw = uxTaskGetStackHighWaterMark(logHandle);
  }
  /* USER CODE END StartReadIMU */
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
    while (count < 10 && osMessageQueueGet(messageQueueHandle, &buffer[count], NULL, osWaitForever) == osOK) {
        count++;
        HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port,  BLUE_LED_Pin);
        osDelay(1);
        HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port,  BLUE_LED_Pin);
        osDelay(1);

    }

    // osStatus_t status = osMessageQueueGet(messageQueueHandle, &buffer, NULL, osWaitForever);
    log_accel(count, buffer, osThreadGetId());
    // HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
    osDelay(1000); // 1Hz, every second
  }
  /* USER CODE END StartLog */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

