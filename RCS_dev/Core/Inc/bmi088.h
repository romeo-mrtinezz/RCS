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

// Addresses-----------------
// Read only
#define GYRO_CHIP_ID         0x00
#define ADDR_RATE_X_LSB      0x02 
#define ADDR_RATE_X_MSB      0x03
#define ADDR_RATE_Y_LSB      0x04
#define ADDR_RATE_Y_MSB      0x05
#define ADDR_RATE_Z_LSB      0x06
#define ADDR_RATE_Z_MSB      0x07
#define GYRO_INT_STAT_1      0x0A

// Read/write
#define GYRO_INT_CTRL 0x15

// Constants----------------------------
#define TIMEOUT 100 //ms
#define BYTE_SIZE 1

// Function declarations


#endif /* __BMI088_H */