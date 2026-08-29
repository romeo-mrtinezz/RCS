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
#include "PID.h"
#include "cmsis_os2.h"
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
#include <inttypes.h>
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
extern GyroData gyro_data;
extern PID_params pid_pitch;
extern PID_params pid_yaw;
extern Attitude attitude;
extern FullData full_data;
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
/* Definitions for AttitudeMutex */
osMutexId_t AttitudeMutexHandle;
const osMutexAttr_t AttitudeMutex_attributes = {
  .name = "AttitudeMutex"
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
   printf("stack overflow occured in task %s\n", pcTaskName);
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
  /* Create the mutex(es) */
  /* creation of AttitudeMutex */
  AttitudeMutexHandle = osMutexNew(&AttitudeMutex_attributes);

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
  messageQueueHandle = osMessageQueueNew (12, sizeof(uint16_t), &messageQueue_attributes);

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
    float accel_pitch, accel_yaw;
    // uint32_t hlw;
    // osStatus_t os_status;
    float prev_pitch = 0, prev_yaw = 0;  
    float curr_pitch, curr_yaw;

  /* Infinite loop */
  for(;;)
  {
    if (received_flag == 1) {
      received_flag = 0;
      if(strncmp((char*)UserRxBufferFS, "open", received_length) == 0) {
        TIM1->CCR1 = 10000; // ARR is 10,000
        HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
        printf("valve opened\n"); }
      else if (strncmp((char*)UserRxBufferFS, "close", received_length) == 0) {
        TIM1->CCR1 = 0;
        HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
        printf("valve closed\n");
      }
    }
        
    // This task is the highest priority task
    // Read IMU
    accel_burst_read(&accel_data);
    gyro_burst_read(&gyro_data); 

    // Estimate euler angles
    accel_to_angle(accel_data, &accel_pitch, &accel_yaw);
    curr_pitch = comp_filter(0.6, 0.1, prev_pitch, -gyro_data.rate_z, accel_pitch);
    curr_yaw = comp_filter(0.6, 0.1, prev_yaw, -gyro_data.rate_y, accel_yaw); // could be gyro z?
    prev_pitch = curr_pitch; 
    prev_yaw = curr_yaw;

    // Write to shared struct safely to pass to PID controller and to log
    osMutexAcquire(AttitudeMutexHandle, osWaitForever);
    // attitude.est_pitch = curr_pitch;
    // attitude.est_yaw = curr_yaw;

    full_data.rate_x = gyro_data.rate_x;
    full_data.rate_y = gyro_data.rate_y;
    full_data.rate_z = gyro_data.rate_z;
    full_data.acc_x = accel_data.acc_x;
    full_data.acc_y = accel_data.acc_y;
    full_data.acc_z = accel_data.acc_z;
    full_data.pitch_accel = accel_pitch;
    full_data.yaw_accel = accel_yaw;
    full_data.pitch = curr_pitch;
    full_data.yaw = curr_yaw;
    osMutexRelease(AttitudeMutexHandle);

    // sprintf(usb_buf, "%lu,%.2f,%.2f,%.2f\n", msg.timestamp, msg.acc_x, msg.acc_y, msg.acc_z);


    osDelay(10); // 100Hz
    // hlw = uxTaskGetStackHighWaterMark(logHandle);
    // printf("%" PRIu32 "\r\n", hlw);
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
  /* init code for USB_Device */
  MX_USB_Device_Init();
  SD_Card_init();
  pid_init(&pid_pitch);
  pid_init(&pid_yaw);

  char usb_buf[100];
  uint8_t usb_status;
  float pitch_duty;
  float yaw_duty;
  // uint32_t hlw;
  // /* Infinite loop */
  for(;;)
  {
    // Pass in attitude struct atomically
    osMutexAcquire(AttitudeMutexHandle, osWaitForever);
    pitch_duty = pid_update(&pid_pitch, 0, full_data.pitch, 0.1);
    yaw_duty = pid_update(&pid_yaw, 0,  full_data.yaw, 0.1);
    full_data.pitch_error = pid_pitch.error;
    full_data.yaw_error = pid_yaw.error;
    full_data.pitch_duty = pitch_duty;
    full_data.yaw_duty = yaw_duty;

    if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
      // usb_status = CDC_Transmit_FS((uint8_t *)usb_buf, strlen(usb_buf)); // OR
      printf("%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\r\n",
        xTaskGetTickCount(),
        full_data.rate_x, full_data.rate_y, full_data.rate_z,
        full_data.acc_x, full_data.acc_y, full_data.acc_z,
        full_data.pitch_accel, full_data.yaw_accel,
        full_data.pitch, full_data.yaw,
        full_data.pitch_error, full_data.yaw_error,
        full_data.pitch_duty, full_data.yaw_duty
      ); // Modify
    }
    osMutexRelease(AttitudeMutexHandle);

    select_thruster(pid_pitch.error, pitch_duty, pid_yaw.error, yaw_duty, 0.1);

    // semaphore here?
    // log_accel(count, buffer, osThreadGetId());
    // post semaphore
    // hlw = uxTaskGetStackHighWaterMark(readIMUHandle);
    // printf("hlw: %" PRIu32 "\r\n", hlw);
    osDelay(100); // 10Hz, every 100ms
  }
  /* USER CODE END StartLog */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

