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
#include "adc.h"
#include "cmsis_os2.h"
#include "stm32g4xx_hal_adc.h"
#include "stm32g4xx_hal_uart.h"
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
extern USBD_HandleTypeDef hUsbDeviceFS;
extern ADC_HandleTypeDef hadc2;

extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
extern uint8_t received_flag;
extern uint32_t received_length;
extern volatile uint8_t rx_flag;
extern char rx_buf[20];
extern char load_cell_dma_buf[20];
extern char load_cell_usb_buf[40];
extern volatile uint8_t load_cell_ready;
extern volatile uint8_t adc_flag;
extern volatile uint8_t rx_load_cell;

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
  /* init code for USB_Device */
  /* USER CODE BEGIN StartReadIMU */
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
  /* Infinite loop */
  uint32_t start = xTaskGetTickCount();
  uint32_t duration;
  for(;;)
  {
    HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc_val, 2);
    high_pt_v = (13.6f/10.0f)*(3.3f/4095.0f)*adc_val[0];
    low_pt_v  = (13.6f/10.0f)*(3.3f/4095.0f)*adc_val[1];
    high_pressure = (high_pt_v - zero_voltage)*(pressure_span/voltage_span) + pressure_offset1;
    low_pressure = (low_pt_v - zero_voltage)*(pressure_span/voltage_span) + pressure_offset2;
    // high_pressure = (high_pt_v - 0.5f)*50.0f + 3.6f; <------- this assumed that we were getting perfect 5V
    // low_pressure  = (low_pt_v  - 0.5f)*50.0f + 3.6f;

    if (load_cell_ready) {
      load_cell_ready = 0;
      snprintf(load_cell_usb_buf, 12, "%.11s", load_cell_dma_buf);
    }

    if (received_flag == 1) { // USB
      received_flag = 0;
      if(strncmp((char*)UserRxBufferFS, "100", received_length) == 0) {
        TIM1->CCR1 = 10000; // ARR is 10,000, currently PWM freq is 10Hz
        // HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
        // EDIT HERE FOR MAPPING LOGIC-------------------------------------------------
        printf("valve opened\n"); 
        osDelay(100); // ms
        TIM1->CCR1 = 0;
        printf("valve closed\n");
      }
      else if (strncmp((char*)UserRxBufferFS, "90", received_length) == 0) {
        TIM1->CCR1 = 9000;
        printf("valve open\n");
        osDelay(100); // ms
        TIM1->CCR1 = 0;
        printf("valve closed\n");
      }
      else if (strncmp((char*)UserRxBufferFS, "80", received_length) == 0) {
        TIM1->CCR1 = 8000;
        printf("valve open\n");
        osDelay(100); // ms
        TIM1->CCR1 = 0;
        printf("valve closed\n");
      }
      else if (strncmp((char*)UserRxBufferFS, "70", received_length) == 0) {
        TIM1->CCR1 = 7000;
        printf("valve open\n");
        osDelay(100); // ms
        TIM1->CCR1 = 0;
        printf("valve closed\n");
      }
      else if (strncmp((char*)UserRxBufferFS, "60", received_length) == 0) {
        TIM1->CCR1 = 6000;
        printf("valve open\n");
        osDelay(100); // ms
        TIM1->CCR1 = 0;
        printf("valve closed\n");
      }
      else if (strncmp((char*)UserRxBufferFS, "50", received_length) == 0) {
        TIM1->CCR1 = 5000; // why didnt this work 
        printf("valve open\n");
        // osDelay(500); // ms 5 cycles
        // TIM1->CCR1 = 0;
        printf("valve closed\n");
      }
      else if (strncmp((char*)UserRxBufferFS, "40", received_length) == 0) {
        TIM1->CCR1 = 4000;
        printf("valve open\n");
        osDelay(100); // ms
        TIM1->CCR1 = 0;
        printf("valve closed\n");
      }
      else if (strncmp((char*)UserRxBufferFS, "30", received_length) == 0) {
        TIM1->CCR1 = 3000;
        printf("valve open\n");
        osDelay(100); // ms
        TIM1->CCR1 = 0;
        printf("valve closed\n");
      }
      else if (strncmp((char*)UserRxBufferFS, "20", received_length) == 0) {
        TIM1->CCR1 = 2000;
        printf("valve open\n");
        osDelay(100); // ms
        TIM1->CCR1 = 0;
        printf("valve closed\n");
      }
      else if (strncmp((char*)UserRxBufferFS, "10", received_length) == 0) {
        TIM1->CCR1 = 1000;
        printf("valve open\n");
        osDelay(100); // ms
        TIM1->CCR1 = 0;
        printf("valve closed\n");
      }
      else if (strncmp((char*)UserRxBufferFS, "open", received_length) == 0) {
        TIM1->CCR1 = 10000;
        printf("valve open\n");
      }
      else if (strncmp((char*)UserRxBufferFS, "close", received_length) == 0) {
        TIM1->CCR1 = 0;
        // HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
        printf("valve closed\n");
      }
    }

    if (adc_flag) { // PT
      HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
      adc_flag = 0;
      duration = xTaskGetTickCount() - start;
      snprintf(pt_buf, 100, "%lu,%.2f,%.2f,%.11s\n", duration, high_pressure, low_pressure, load_cell_usb_buf);
      // if above truncates text, most likely buffer overflow because duration gets to a 6 digit string

      CDC_Transmit_FS((uint8_t *)pt_buf, strlen(pt_buf));
      // HAL_UART_Transmit(&huart4, (uint8_t *)pt_buf, strlen(pt_buf), 50);
    }
     
    osDelay(17);
  /* USER CODE END StartReadIMU */
  }
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
  // /* Infinite loop */
  for(;;)
  {
    osDelay(10000); // 10Hz, every 100ms
  }
  /* USER CODE END StartLog */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

