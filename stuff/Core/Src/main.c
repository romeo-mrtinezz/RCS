/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "app_fatfs.h"
#include "spi.h"
#include "tim.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bmi088.h"
#include "ff.h"
#include "pid.h"

#include "stm32g483xx.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_spi.h"
#include <stdint.h>
#include <stdio.h>

#include <stdbool.h>
#include <string.h>
#include <math.h>

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

/* USER CODE BEGIN PV */
uint8_t byte_2;
char TxBuffer[250]; // 250 bytes, char is 1 byte

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void run_blinky() {
  // Use function in while loop
  HAL_GPIO_WritePin(GPIOA, RED_LED_Pin, GPIO_PIN_SET);
  HAL_Delay(1000);
  HAL_GPIO_WritePin(GPIOA, RED_LED_Pin, GPIO_PIN_RESET);
  HAL_Delay(1000);
};
 
void gyro_spi_read(uint8_t address) {
  // Calculate byte 1 from address and R command
  uint8_t byte_1 = address | 0x80;

  // 1. Send address and R command
  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_RESET); // Chip select low
  // HAL_Delay(100);
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT); // Send byte 1
  // HAL_Delay(100);

  // 2. Read data
  HAL_SPI_Receive(&hspi1, &byte_2, BYTE_SIZE, TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_SET);  // End comms
  // don't have to return anything because we passed in address of byte 2
};


uint8_t initialise_accel() {
  uint8_t byte_1 = ACC_PWR_CTRL | 0x00; // Write mode
  uint8_t pwr_set = 0x04;
  uint8_t accel_id;
  uint8_t dummy;

  // 1. Switch to spi mode
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_SET); // rising edge to switch to spi
  // HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_SET); // deselect gyro

  // 2. Switch from suspend mode to normal power mode
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT); // Send byte 1
  HAL_SPI_Transmit(&hspi1, &pwr_set, BYTE_SIZE, TIMEOUT); // Send byte 1
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_SET); 
  HAL_Delay(1);

  // 3. Read accel id
  byte_1 = ACC_CHIP_ID | 0x80;
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_RESET); // pull cs low
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT); // Send byte 1
  HAL_SPI_Receive(&hspi1, &dummy, BYTE_SIZE, TIMEOUT); // dummy
  HAL_SPI_Receive(&hspi1, &accel_id, BYTE_SIZE, TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_SET); // end transaction
  
  return accel_id; // expect 0x1E = 30 
};

uint8_t accel_spi_read(uint8_t address) {
  uint8_t result;
  uint8_t dummy;

  uint8_t byte_1 = address | 0x80; // R mode
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_RESET); // pull cs low
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT); // Send byte 1
  HAL_SPI_Receive(&hspi1, &dummy, BYTE_SIZE, TIMEOUT); // dummy
  HAL_SPI_Receive(&hspi1, &result, BYTE_SIZE, TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_SET); // end transaction
  
  return result; 
};

/** 
  * @brief read all 3 axis at once
  * @param first_address
  * @retval None
*/
AccData accel_burst_read(uint8_t first_address) {
  uint8_t dummy;
  uint8_t byte_1 = first_address | 0x80; // R mode
  uint8_t acc_buffer[6]; // 6 bytes of data
  AccData accel_data;

  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_RESET); // pull cs low
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT); // Send byte 1
  HAL_SPI_Receive(&hspi1, &dummy, BYTE_SIZE, TIMEOUT); // dummy
  HAL_SPI_Receive(&hspi1, acc_buffer, sizeof(acc_buffer), TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_SET); // end transaction

  // cast to int due to 2's complement
  // default acc_range 0x01, +-6g
  // Accel in mg
  accel_data.acc_x = (int16_t)(acc_buffer[1] << 8 | acc_buffer[0])/32768.0f * 1000.0f * 4.0f * 1.5f; // msb*256+lsb 
  accel_data.acc_y = (int16_t)(acc_buffer[3] << 8 | acc_buffer[2])/32768.0f * 1000.0f * 4.0f * 1.5f;
  accel_data.acc_z = (int16_t)(acc_buffer[5] << 8 | acc_buffer[4])/32768.0f * 1000.0f * 4.0f * 1.5f;

  return accel_data;
};

GyroData gyro_burst_read(uint8_t first_address) {
  uint8_t byte_1 = first_address | 0x80; // R mode
  uint8_t gyro_buffer[6]; // 6 bytes of data
  GyroData gyro_data;

  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT);
  HAL_SPI_Receive(&hspi1, gyro_buffer, sizeof(gyro_buffer), TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_SET);

  gyro_data.rate_x = (int16_t)(gyro_buffer[1] << 8 | gyro_buffer[0]) * 0.061f; // deg/sec
  gyro_data.rate_y = (int16_t)(gyro_buffer[3] << 8 | gyro_buffer[2]) * 0.061f;
  gyro_data.rate_z = (int16_t)(gyro_buffer[5] << 8 | gyro_buffer[4]) * 0.061f;

  return gyro_data;
};

void pwm_logic(float acc_y) {

  if (acc_y > 200 && acc_y <= 450) {
    TIM1->CCR1 = 0; 
    TIM1->CCR2 = 2500;     
  }
  else if (acc_y > 450) {
    TIM1->CCR1 = 0; 
    TIM1->CCR2 = 5000;     
  }
  else if (acc_y <= 200 && acc_y >= -200) {
    TIM1->CCR1 = 0;
    TIM1->CCR2 = 0;   
  }
  else if (acc_y < -200 && acc_y >= -450) {
    TIM1->CCR1 = 2500; 
    TIM1->CCR2 = 0; 
  }
  else if (acc_y < -450) {
    TIM1->CCR1 = 5000; 
    TIM1->CCR2 = 0; 
  }

}

void accel_to_angle(AccData accel_data, float * accel_pitch, float * accel_yaw) {
  *accel_pitch = (float)atan2(accel_data.acc_x, accel_data.acc_z); // shouldn't matter if in mg
  *accel_yaw = (float)atan2(accel_data.acc_y, accel_data.acc_z); // radians
  
  *accel_pitch = *accel_pitch * 180.0f/M_PI; // degrees
  *accel_yaw = *accel_yaw * 180.0f/M_PI; 
};

static void SD_Card_Write(void) {
  FATFS FatFs;
  FIL Fil;
  FRESULT FR_Status;
  FATFS *FS_Ptr;
  UINT RWC, WWC; // Read/Write Word Counter
  DWORD FreeClusters;
  uint32_t TotalSize, FreeSpace;
  char RW_Buffer[200]; 
  
  do {
    // Mount SD Card
    FR_Status = f_mount(&FatFs, "", 1);
    if (FR_Status != FR_OK)
    {
      sprintf(TxBuffer, "Error! While Mounting SD Card, Error Code: (%i)\r\n", FR_Status);
      break;
    }
    sprintf(TxBuffer, "SD Card Mounted Successfully! \r\n\n");

    // Get SD card size and free space
    f_getfree("", &FreeClusters, &FS_Ptr);
    TotalSize = (uint32_t)((FS_Ptr->n_fatent - 2) * FS_Ptr->csize * 0.5);
    FreeSpace = (uint32_t)(FreeClusters * FS_Ptr->csize * 0.5);

    // Create and open csv file
    FR_Status = f_open(&Fil, "test.csv", FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
    if(FR_Status != FR_OK)
    {
      sprintf(TxBuffer, "Error! While Creating/Opening A New Text File, Error Code: (%i)\r\n", FR_Status);
      break;
    }

    // Write headers
    f_puts("time_ms,acc_x,acc_y,acc_z\n", &Fil); //\n move cursor to front, new line

    // Write data, sprintf to convert string to floats
    sprintf(RW_Buffer, "%.1d, %.2f, %.2f, %.2f\n", 34, 12.3, 1.5, 4.7);
    f_write(&Fil, RW_Buffer, strlen(RW_Buffer), &WWC);

    // Close file, else last chunk of data doesnt get flushed, file size stays at 0 etc. data corrupted
    f_close(&Fil);
  } while(0);

  // Unmount SD card
  FR_Status = f_mount(NULL, "", 0);
  
}

static void SD_Card_Test(void) { // static means can only be used in this file, you can have the same name function in other files
  FATFS FatFs;
  FIL Fil;
  FRESULT FR_Status;
  FATFS *FS_Ptr;
  UINT RWC, WWC; // Read/Write Word Counter
  DWORD FreeClusters;
  uint32_t TotalSize, FreeSpace;
  char RW_Buffer[200];
  do
  {
    //------------------[ Mount The SD Card ]--------------------
    FR_Status = f_mount(&FatFs, "", 1);
    if (FR_Status != FR_OK)
    {
      sprintf(TxBuffer, "Error! While Mounting SD Card, Error Code: (%i)\r\n", FR_Status);
      // UART_Print(TxBuffer);
      break;
    }
    sprintf(TxBuffer, "SD Card Mounted Successfully! \r\n\n");
    // UART_Print(TxBuffer);
    //------------------[ Get & Print The SD Card Size & Free Space ]--------------------
    f_getfree("", &FreeClusters, &FS_Ptr);
    TotalSize = (uint32_t)((FS_Ptr->n_fatent - 2) * FS_Ptr->csize * 0.5);
    FreeSpace = (uint32_t)(FreeClusters * FS_Ptr->csize * 0.5);
    sprintf(TxBuffer, "Total SD Card Size: %lu Bytes\r\n", TotalSize);
    // UART_Print(TxBuffer);
    sprintf(TxBuffer, "Free SD Card Space: %lu Bytes\r\n\n", FreeSpace);
    // UART_Print(TxBuffer);
    //------------------[ Open A Text File For Write & Write Data ]--------------------
    //Open the file
    FR_Status = f_open(&Fil, "TextFileWrite.txt", FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
    if(FR_Status != FR_OK)
    {
      sprintf(TxBuffer, "Error! While Creating/Opening A New Text File, Error Code: (%i)\r\n", FR_Status);
      // UART_Print(TxBuffer);
      break;
    }
    sprintf(TxBuffer, "Text File Created & Opened! Writing Data To The Text File..\r\n\n");
    // UART_Print(TxBuffer);
    // (1) Write Data To The Text File [ Using f_puts() Function ]
    f_puts("Hello! From STM32 To SD Card Over SPI, Using f_puts()\n", &Fil);
    // (2) Write Data To The Text File [ Using f_write() Function ]
    strcpy(RW_Buffer, "Hello! From STM32 To SD Card Over SPI, Using f_write()\r\n");
    f_write(&Fil, RW_Buffer, strlen(RW_Buffer), &WWC);
    // Close The File
    f_close(&Fil);
    //------------------[ Open A Text File For Read & Read Its Data ]--------------------
    // Open The File
    FR_Status = f_open(&Fil, "TextFileWrite.txt", FA_READ);
    if(FR_Status != FR_OK)
    {
      sprintf(TxBuffer, "Error! While Opening (TextFileWrite.txt) File For Read.. \r\n");
      // UART_Print(TxBuffer);
      break;
    }
    // (1) Read The Text File's Data [ Using f_gets() Function ]
    f_gets(RW_Buffer, sizeof(RW_Buffer), &Fil);
    sprintf(TxBuffer, "Data Read From (TextFileWrite.txt) Using f_gets():%s", RW_Buffer);
    // UART_Print(TxBuffer);
    // (2) Read The Text File's Data [ Using f_read() Function ]
    f_read(&Fil, RW_Buffer, f_size(&Fil), &RWC);
    sprintf(TxBuffer, "Data Read From (TextFileWrite.txt) Using f_read():%s", RW_Buffer);
    // UART_Print(TxBuffer);
    // Close The File
    f_close(&Fil);
    sprintf(TxBuffer, "File Closed! \r\n\n");
    // UART_Print(TxBuffer);
    //------------------[ Open An Existing Text File, Update Its Content, Read It Back ]--------------------
    // (1) Open The Existing File For Write (Update)
    FR_Status = f_open(&Fil, "TextFileWrite.txt", FA_OPEN_EXISTING | FA_WRITE);
    FR_Status = f_lseek(&Fil, f_size(&Fil)); // Move The File Pointer To The EOF (End-Of-File)
    if(FR_Status != FR_OK)
    {
      sprintf(TxBuffer, "Error! While Opening (TextFileWrite.txt) File For Update.. \r\n");
      // UART_Print(TxBuffer);
      break;
    }
    // (2) Write New Line of Text Data To The File
    FR_Status = f_puts("This New Line Was Added During Update!\r\n", &Fil);
    f_close(&Fil);
    memset(RW_Buffer,'\0',sizeof(RW_Buffer)); // Clear The Buffer
    // (3) Read The Contents of The Text File After The Update
    FR_Status = f_open(&Fil, "TextFileWrite.txt", FA_READ); // Open The File For Read
    f_read(&Fil, RW_Buffer, f_size(&Fil), &RWC);
    sprintf(TxBuffer, "Data Read From (TextFileWrite.txt) After Update:%s", RW_Buffer);
    // UART_Print(TxBuffer);
    f_close(&Fil);
    //------------------[ Delete The Text File ]--------------------
    // Delete The File
    /*
    FR_Status = f_unlink(TextFileWrite.txt);
    if (FR_Status != FR_OK){
        sprintf(TxBuffer, "Error! While Deleting The (TextFileWrite.txt) File.. \r\n");
        // UART_Print(TxBuffer);
    }
    */
  } while(0); // keep false so program continues
  //------------------[ Test Complete! Unmount The SD Card ]--------------------
  FR_Status = f_mount(NULL, "", 0);
  if (FR_Status != FR_OK)
  {
      sprintf(TxBuffer, "Error! While Un-mounting SD Card, Error Code: (%i)\r\n", FR_Status);
      // UART_Print(TxBuffer);
  } else{
      sprintf(TxBuffer, "SD Card Un-mounted Successfully! \r\n");
      // UART_Print(TxBuffer);
  }
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_USB_PCD_Init();
  if (MX_FATFS_Init() != APP_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

  TIM1->CCR1 = 0; // 50% duty cycle for ARR = 10,000
  TIM1->CCR2 = 0;

  byte_2 = initialise_accel();
  AccData accel_data;
  GyroData gyro_data;

  // SD_Card_Test();
  // SD_Card_Write();
  float accel_pitch, accel_yaw;
  float prev_pitch = 0, prev_yaw = 0;
  PID_params pid;
  pid_init(&pid); // bro check ur dereferencing
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  { 
    accel_data = accel_burst_read(ACC_X_LSB);
    gyro_data = gyro_burst_read(ADDR_RATE_X_LSB);
    
    accel_to_angle(accel_data, &accel_pitch, &accel_yaw);
    float pitch = comp_filter(0, 1, prev_pitch, gyro_data.rate_x, accel_pitch); // alpha = 1 means purely based on accel
    uint16_t pitch_duty = pid_update(&pid, 0, pitch, 1); 
    select_thruster(pid.error, pitch_duty, 0, 0, 1);
    
    // pwm_logic(accel_data.acc_y);
    
    // gyro_spi_read(GYRO_CHIP_ID); 

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
