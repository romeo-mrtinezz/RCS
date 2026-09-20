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

// RFD
extern volatile uint8_t rfd_rx_flag;

// Load cell
extern char load_cell_dma_buf[20];
extern char load_cell_usb_buf[40];
extern volatile uint8_t rx_load_cell;
extern volatile uint8_t load_cell_ready;

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
    osDelay(1);
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
  /* Infinite loop */

  for(;;)
  {
    // high_pressure = (high_pt_v - 0.5f)*50.0f + 3.6f; <------- this assumed that we were getting perfect 5V
    // low_pressure  = (low_pt_v  - 0.5f)*50.0f + 3.6f;

    if (load_cell_ready) {
      load_cell_ready = 0;
      snprintf(load_cell_usb_buf, 12, "%.11s", load_cell_dma_buf);
    }

    if (received_flag == 1) { // USB
      received_flag = 0;
      if(strncmp((char*)UserRxBufferFS, "open", received_length) == 0) {
        TIM1->CCR1 = 10000; // ARR is 10,000, currently PWM freq is 10Hz
        // HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
        // EDIT HERE FOR MAPPING LOGIC-------------------------------------------------
        printf("valve opened\n"); 
        osDelay(100); // ms
        TIM1->CCR1 = 0;
        printf("valve closed\n");
      }
      else if (strncmp((char*)UserRxBufferFS, "close", received_length) == 0) {
        TIM1->CCR1 = 0;
        // HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
        printf("valve closed\n");
      }
    }
     
    osDelay(14);
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
  volatile uint16_t adc_val[2];
  float high_pressure;
  float low_pressure;
  float high_pt_v;
  float low_pt_v;
  char pt_buf[100];

  float supply_voltage = 4.97; // 11.74V battery, configurable CHANGE TO 4.97 for USB, 4.84 for battery only
  float zero_voltage = 0.1*supply_voltage;
  float full_scale_voltage = 0.9*supply_voltage;
  float voltage_span = full_scale_voltage-zero_voltage;
  float pressure_span = 200; // 0-200 bar PT
  float pressure_offset1 = 3.8; // Constant offset if needed
  float pressure_offset2 = 3.8;
  uint32_t start = xTaskGetTickCount();
  uint32_t duration;
  /* Infinite loop */
  for(;;)
  {
    HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc_val, 2);
    high_pt_v = (13.6f/10.0f)*(3.3f/4095.0f)*adc_val[0];
    low_pt_v  = (13.6f/10.0f)*(3.3f/4095.0f)*adc_val[1];
    high_pressure = (high_pt_v - zero_voltage)*(pressure_span/voltage_span) + pressure_offset1;
    low_pressure = (low_pt_v - zero_voltage)*(pressure_span/voltage_span) + pressure_offset2;
    //

    if (pt_adc_flag) { // PT
      HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
      pt_adc_flag = 0;
      duration = xTaskGetTickCount() - start;
      snprintf(pt_buf, 100, "%lu,%.2f,%.2f,%.11s\n", duration, high_pressure, low_pressure, load_cell_usb_buf);
      // if above truncates text, most likely buffer overflow because duration gets to a 6 digit string

      // CDC_Transmit_FS((uint8_t *)pt_buf, strlen(pt_buf));
      HAL_UART_Transmit(&huart4, (uint8_t *)pt_buf, strlen(pt_buf), 50);
    }
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

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

