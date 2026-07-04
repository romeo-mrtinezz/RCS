/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bmi088.h
  * @brief          : Header for bmi088.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  *
  ******************************************************************************
  */
/* USER CODE END Header */

// Define addresses

/*
PS Pin grounded for SPI for gyro
Send HIGH to CSB1 pin of accel to set to SPI

Enter normal mode for accel by writing 4 to ACC_PWR_CTRL

Device resets to default settings after every power on. 
gyro and accel data are 16 bit wide

can configure data rates and filter params

setup interrupts

enable fifo

COMMS with sensor-------------
Registers width of 8 bits

burst access mechanism which auto-increments the register being read without having to increment in code

*/
#ifndef BMI088_H
#define BMI088_H

// Includes
#include "main.h"
#include "spi.h"
#include "stm32g483xx.h"
#include "stm32g4xx_hal_spi.h"
extern SPI_HandleTypeDef hspi1;

// Gyroscope register map-----------------
// Read only
#define GYRO_CHIP_ID         0x00
#define RATE_X_LSB           0x02 
#define RATE_X_MSB           0x03
#define RATE_Y_LSB           0x04
#define RATE_Y_MSB           0x05
#define RATE_Z_LSB           0x06
#define RATE_Z_MSB           0x07
#define GYRO_INT_STAT_1      0x0A
#define FIFO_DATA            0x3F
#define FIFO_STATUS          0x0E

// Read/write
#define FIFO_CONFIG_1 0x3E
#define GYRO_INT_CTRL 0x15

// Accelerometer register map
// Read only
#define ACC_CHIP_ID         0x00
#define ACC_X_LSB           0x12
#define ACC_X_MSB           0x13
#define ACC_Y_LSB           0x14
#define ACC_Y_MSB           0x15
#define ACC_Z_LSB           0x16
#define ACC_Z_MSB           0x17

// Read/write
#define ACC_PWR_CTRL        0x7D

// Constants
#define TIMEOUT 100 //ms
#define BYTE_SIZE 1

// Functions
uint8_t gyro_spi_read(uint8_t address);
uint8_t accel_init();
uint8_t accel_spi_read(uint8_t address);
void accel_burst_read(AccData * accel_data);
void gyro_burst_read(GyroData * gyro_data);


#endif /* __BMI088_H */