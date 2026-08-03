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
#include "stm32g4xx_hal.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pid.h"
#include "bmi088.h"
#include "sd.h"
#include "global.h"
#include <stdint.h>
#include <string.h>
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "usbd_def.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern USBD_HandleTypeDef hUsbDeviceFS;

extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
extern uint8_t received_flag;
extern uint32_t received_length;
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
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 2500 * 4
};
/* Definitions for log */
osThreadId_t logHandle;
const osThreadAttr_t log_attributes = {
  .name = "log",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 2000 * 4
};
/* Definitions for messageQueue */
osMessageQueueId_t messageQueueHandle;
const osMessageQueueAttr_t messageQueue_attributes = {
  .name = "messageQueue"
};
/* Definitions for testSemaphore */
osSemaphoreId_t testSemaphoreHandle;
const osSemaphoreAttr_t testSemaphore_attributes = {
  .name = "testSemaphore"
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

  /* Create the semaphores(s) */
  /* creation of testSemaphore */
  testSemaphoreHandle = osSemaphoreNew(1, 1, &testSemaphore_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of messageQueue */
  // messageQueueHandle = osMessageQueueNew (12, sizeof(uint16_t), &messageQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  messageQueueHandle = osMessageQueueNew (12, sizeof(MessageQueue_t), &messageQueue_attributes);

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
  /* init code for USB_Device */
  MX_USB_Device_Init();
  for(;;) {
    if (received_flag == 1) {
      received_flag = 0;
      if(strncmp((char*)UserRxBufferFS, "ping", received_length) == 0) {
        HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
        printf("pong\n");
      }
    }
    // printf("hey\n");
  // /* USER CODE BEGIN StartReadIMU */
  //   MessageQueue_t msg;
  //   // float accel_pitch, accel_yaw;
  //   // uint32_t hlw;
  //   char usb_buf[100];
  //   // osStatus_t os_status;
  //   uint8_t usb_status;

  // /* Infinite loop */
  // for(;;)
  // {
  //   // This task is the highest priority though, so I assume no other task would prempt it?
  //   accel_burst_read(&accel_data);
  //   msg.timestamp = xTaskGetTickCount(); // uint32? 
  //   // accel_to_angle(accel_data, &accel_pitch, &accel_yaw);
  //   msg.acc_x = accel_data.acc_x;
  //   msg.acc_y = accel_data.acc_y;
  //   msg.acc_z = accel_data.acc_z;

  //   // sprintf(usb_buf, "Hey world\n");
  //   sprintf(usb_buf, "Time: %lums | acc_x:%.2f | acc_y:%.2f | acc_z:%.2f\n", msg.timestamp, accel_data.acc_x, accel_data.acc_y, accel_data.acc_z);
  //   if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
  //     usb_status = CDC_Transmit_FS((uint8_t *)usb_buf, strlen(usb_buf));
  //   }
  //   osMessageQueuePut(messageQueueHandle, &msg, 0, 0);
  //   // HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port,  BLUE_LED_Pin);
  //   // osDelay(1);
  //   // HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port,  BLUE_LED_Pin);
  //   // osDelay(1);    
    osDelay(100); // 10Hz
    // hlw = uxTaskGetStackHighWaterMark(logHandle);
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
  // SD_Card_init();

  /* Infinite loop */
  for(;;)
  {
    // 0 timeout means will return immediately - I want this because I want to maintain the 1Hz
    // There are 12 elements in buffer, so there should always be 10 in there anyways since I'm reading at 10Hz
    uint8_t count = 0;
    // This ensures that the program doesnt interrup the usb tranmsit and get stuck waiting for queue to filled up
    while (count < 10 && osMessageQueueGet(messageQueueHandle, &buffer[count], NULL, osWaitForever) == osOK) {
        count++;
        HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port,  BLUE_LED_Pin);
        osDelay(1);
        HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port,  BLUE_LED_Pin);
        osDelay(1);

    }
    // semaphore here?
    // log_accel(count, buffer, osThreadGetId());
    // post semaphore, this is so 
    osDelay(1000); // 1Hz, every second
  }
  /* USER CODE END StartLog */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

