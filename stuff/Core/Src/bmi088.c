/**
  ******************************************************************************
  * @file           : bmi088.c
  * @brief          : This file contains the functions to interface with the sensor over SPI
  *               
  ******************************************************************************
  *
  ******************************************************************************
  */
/* USER CODE END Header */

// Includes
#include "main.h"
#include "bmi088.h"
#include "stm32g4xx_hal_spi.h"
#include <stdint.h>
// extern SPI_HandleTypeDef hspi1;

// Functions
uint8_t gyro_spi_read(uint8_t address) {
  // Calculate byte 1 from address and R command
  uint8_t byte_1 = address | 0x80;
  uint8_t data_byte;

  // 1. Send address and R command
  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_RESET); // Chip select low
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT); // Send byte 1

  // 2. Read data
  HAL_SPI_Receive(&hspi1, &data_byte, BYTE_SIZE, TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_SET);  // End comms

  return data_byte;
};


uint8_t accel_init() {
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
  * @param first_address // ACC_X_LSB I think
  * @retval None
*/
void accel_burst_read(uint8_t first_address, AccData * accel_data) { 
  uint8_t dummy;
  uint8_t byte_1 = first_address | 0x80; // R mode
  uint8_t acc_buffer[6]; // 6 bytes of data

  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_RESET); // pull cs low
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT); // Send byte 1
  HAL_SPI_Receive(&hspi1, &dummy, BYTE_SIZE, TIMEOUT); // dummy
  HAL_SPI_Receive(&hspi1, acc_buffer, sizeof(acc_buffer), TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_ACCEL_Pin, GPIO_PIN_SET); // end transaction

  // cast to int due to 2's complement
  // default acc_range 0x01, +-6g
  // Accel in mg
  accel_data->acc_x = (int16_t)(acc_buffer[1] << 8 | acc_buffer[0])/32768.0f * 1000.0f * 4.0f * 1.5f; // msb*256+lsb 
  accel_data->acc_y = (int16_t)(acc_buffer[3] << 8 | acc_buffer[2])/32768.0f * 1000.0f * 4.0f * 1.5f;
  accel_data->acc_z = (int16_t)(acc_buffer[5] << 8 | acc_buffer[4])/32768.0f * 1000.0f * 4.0f * 1.5f;
};

void gyro_burst_read(uint8_t first_address, GyroData * gyro_data) {
  uint8_t byte_1 = first_address | 0x80; // R mode
  uint8_t gyro_buffer[6]; // 6 bytes of data

  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, &byte_1, BYTE_SIZE, TIMEOUT);
  HAL_SPI_Receive(&hspi1, gyro_buffer, sizeof(gyro_buffer), TIMEOUT);
  HAL_GPIO_WritePin(GPIOC, CS_GYRO_Pin, GPIO_PIN_SET);

  gyro_data->rate_x = (int16_t)(gyro_buffer[1] << 8 | gyro_buffer[0]) * 0.061f; // deg/sec
  gyro_data->rate_y = (int16_t)(gyro_buffer[3] << 8 | gyro_buffer[2]) * 0.061f;
  gyro_data->rate_z = (int16_t)(gyro_buffer[5] << 8 | gyro_buffer[4]) * 0.061f;
};