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
#include "PID.h"
#include "cmsis_os2.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "pid.h"
#include "bmi088.h"
#include "sd.h"
#include "global.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "usbd_def.h"
#include <inttypes.h>
#include <sys/_intsup.h>
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
// Global attitude-related structs -------------------------
extern AccData accel_data;
extern GyroData gyro_data;
extern PID_params pid_pitch;
extern PID_params pid_yaw;
extern Attitude attitude;
extern FullData full_data;

// extern USBD_HandleTypeDef hUsbDeviceFS;
extern ADC_HandleTypeDef hadc2;

// USB variables, from usbd_cdc_if.h-----------------------
extern uint8_t received_flag;
extern uint32_t received_length;
extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

// Pressure ----------------------------------------------
extern volatile uint8_t pt_adc_flag;
extern volatile char pt_dma_buf[20];

// RFD ------------------------------------------------------
extern volatile uint8_t rfd_rx_flag;
char rfd_buf[100];

// Load cell -----------------------------------------------
extern char load_cell_dma_buf[20];
extern char load_cell_usb_buf[40];
extern volatile uint8_t rx_load_cell;
extern volatile uint8_t load_cell_ready;

// PID ----------------------------
float pitch_duty, yaw_duty;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 200 * 4
};
/* Definitions for controlLoop */
osThreadId_t controlLoopHandle;
const osThreadAttr_t controlLoop_attributes = {
  .name = "controlLoop",
  .priority = (osPriority_t) osPriorityHigh7,
  .stack_size = 2000 * 4
};
/* Definitions for readImu */
osThreadId_t readImuHandle;
const osThreadAttr_t readImu_attributes = {
  .name = "readImu",
  .priority = (osPriority_t) osPriorityHigh6,
  .stack_size = 2000 * 4
};
/* Definitions for stream */
osThreadId_t streamHandle;
const osThreadAttr_t stream_attributes = {
  .name = "stream",
  .priority = (osPriority_t) osPriorityNormal4,
  .stack_size = 2000 * 4
};
/* Definitions for log */
osThreadId_t logHandle;
const osThreadAttr_t log_attributes = {
  .name = "log",
  .priority = (osPriority_t) osPriorityNormal5,
  .stack_size = 2000 * 4
};
/* Definitions for userMenu */
osThreadId_t userMenuHandle;
const osThreadAttr_t userMenu_attributes = {
  .name = "userMenu",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 2000 * 4
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

void StartDefaultTask(void *argument);
void StartControlLoop(void *argument);
void StartReadIMU(void *argument);
void StartStream(void *argument);
void StartLog(void *argument);
void StartUserMenu(void *argument);

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

  /* USER CODE BEGIN RTOS_QUEUES */

  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of controlLoop */
  controlLoopHandle = osThreadNew(StartControlLoop, NULL, &controlLoop_attributes);

  /* creation of readImu */
  readImuHandle = osThreadNew(StartReadIMU, NULL, &readImu_attributes);

  /* creation of stream */
  streamHandle = osThreadNew(StartStream, NULL, &stream_attributes);

  /* creation of log */
  logHandle = osThreadNew(StartLog, NULL, &log_attributes);

  /* creation of userMenu */
  userMenuHandle = osThreadNew(StartUserMenu, NULL, &userMenu_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_Device */
  MX_USB_Device_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartControlLoop */
/**
* @brief Function implementing the controlLoop thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartControlLoop */
void StartControlLoop(void *argument)
{
  /* USER CODE BEGIN StartControlLoop */
  /* Infinite loop */
  for(;;)
  {
    osDelay(200); // 5 Hz, 200ms
  }
  /* USER CODE END StartControlLoop */
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
  // HIGHEST PRIORITY TASK
  float accel_pitch, accel_yaw;
  float prev_pitch = 0, prev_yaw = 0;
  float curr_pitch, curr_yaw;
  /* Infinite loop */
  for(;;)
  {
    accel_burst_read(&accel_data);
    gyro_burst_read(&gyro_data);
    
    accel_to_angle(accel_data, &accel_pitch, &accel_yaw);
    curr_pitch = comp_filter(0.6, 0.1, prev_pitch, -gyro_data.rate_z, accel_pitch); // change dt?
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

    osDelay(10); // 100 Hz, 10ms
  /* USER CODE END StartReadIMU */
  }
}
/* USER CODE BEGIN Header_StartStream */
/**
* @brief Function implementing the stream thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartStream */
void StartStream(void *argument)
{
  /* USER CODE BEGIN StartStream */
  /* Infinite loop */
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

    // Copy buffer data into buffer for rfd
    sprintf(rfd_buf, "%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\r\n",
        xTaskGetTickCount(),
        full_data.rate_x, full_data.rate_y, full_data.rate_z,
        full_data.acc_x, full_data.acc_y, full_data.acc_z,
        full_data.pitch_accel, full_data.yaw_accel,
        full_data.pitch, full_data.yaw,
        full_data.pitch_error, full_data.yaw_error,
        full_data.pitch_duty, full_data.yaw_duty
      );

    // Send over rfd
    HAL_UART_Transmit(&huart4, (uint8_t *)rfd_buf, strlen(rfd_buf), 100);
    osMutexRelease(AttitudeMutexHandle);
    osDelay(100); // 10Hz, 100ms
  }
  /* USER CODE END StartStream */
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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartLog */
}

/* USER CODE BEGIN Header_StartUserMenu */
/**
* @brief Function implementing the userMenu thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUserMenu */
void StartUserMenu(void *argument)
{
  /* USER CODE BEGIN StartUserMenu */
  /* Infinite loop */
  for(;;)
  {
    // if (received_flag == 1) { // USB
    //   received_flag = 0;
    //   if(strncmp((char*)UserRxBufferFS, "open", received_length) == 0) {
    //     TIM1->CCR1 = 10000; // ARR is 10,000, currently PWM freq is 10Hz
    //     printf("valve opened\n"); 
    //     osDelay(100); // ms
    //     TIM1->CCR1 = 0;
    //     printf("valve closed\n");
    //   }
    //   else if (strncmp((char*)UserRxBufferFS, "close", received_length) == 0) {
    //     TIM1->CCR1 = 0;
    //     // HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
    //     printf("valve closed\n");
    //   }
    // }
    osDelay(1);
  }
  /* USER CODE END StartUserMenu */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

