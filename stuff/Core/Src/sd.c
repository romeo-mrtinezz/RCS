
// Includes
#include "global.h"
#include "app_fatfs.h"
#include "cmsis_os2.h"
#include "ff.h"
#include "main.h"
#include "projdefs.h"
#include <stdint.h>
#include <sys/_intsup.h>
#include <stdio.h>
#include <string.h>
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_gpio.h"

void SD_Card_Write() {
  FRESULT FR_Status;
  FATFS *FS_Ptr;
  UINT RWC, WWC; // Read/Write Word Counter
  DWORD FreeClusters;
  uint32_t TotalSize, FreeSpace;
  char RW_Buffer[200];
  char TxBuffer[250]; 
  
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

void SD_Card_Test() { // static means can only be used in this file, you can have the same name function in other files
  FRESULT FR_Status;
  FATFS *FS_Ptr;
  UINT RWC, WWC; // Read/Write Word Counter
  DWORD FreeClusters;
  uint32_t TotalSize, FreeSpace;
  char RW_Buffer[200];
  char TxBuffer[250];
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

/*
Create csv file
Mount sd card
*/
void SD_Card_init() {
  FRESULT FR_Status;
  FATFS *FS_Ptr;
  UINT RWC, WWC; // Read/Write Word Counter
  DWORD FreeClusters;
  uint32_t TotalSize, FreeSpace;
  char TxBuffer[250]; 
  
  do {
    // Mount SD card
    FR_Status = f_mount(&FatFs, "", 0);
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

    // Open csv file, if doesn't exist, creates it
    FR_Status = f_open(&Fil, "accel_data.csv", FA_WRITE | FA_READ | FA_OPEN_ALWAYS);
    if(FR_Status != FR_OK)
    {
        sprintf(TxBuffer, "Error! While Creating/Opening A New Text File, Error Code: (%i)\r\n", FR_Status);
        break;
    }

    // Write headers
    f_puts("time_ms,acc_x,acc_y,acc_z\n", &Fil); //\n move cursor to front, new line
    // f_lseek(&Fil, f_size(&Fil)) // look for end of file
    f_sync(&Fil);
    } while(0);
}

void log_accel(uint8_t count, MessageQueue_t * buffer, osThreadId_t thread_id) {
  FRESULT FR_Status;
  FATFS *FS_Ptr;
  UINT RWC, WWC; // Read/Write Word Counter
  char TxBuffer[250]; 
  char RW_Buffer[200];

do {
    // Write 10 elements to sd card
    for (uint8_t i = 0; i < count; i++) {
      sprintf(RW_Buffer, "%lu,%.2f,%.2f,%.2f\n",
              buffer[i].timestamp, buffer[i].acc_x, buffer[i].acc_y, buffer[i].acc_z);
      f_write(&Fil, RW_Buffer, strlen(RW_Buffer), &WWC); 
      }

    f_sync(&Fil);
    

    if (xTaskGetTickCount() > pdMS_TO_TICKS(20000)) { // after 20s dismount sdcard and end task
      // Unmount SD card
      f_close(&Fil);
      FR_Status = f_mount(NULL, "", 0);
      HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
      osThreadTerminate(thread_id);
    }
  } while(0);
}

// no rtos
void log_pls(uint32_t time, AccData * accel_data) {
  FRESULT FR_Status;
  FATFS *FS_Ptr;
  UINT RWC, WWC; // Read/Write Word Counter
  char TxBuffer[250]; 
  char RW_Buffer[200]; 
  sprintf(RW_Buffer, "%lu,%.2f,%.2f,%.2f\n", time, accel_data->acc_x, accel_data->acc_y, accel_data->acc_z);
  f_write(&Fil, RW_Buffer, strlen(RW_Buffer), &WWC); 
  if (time >= 20000) { //ms
    f_close(&Fil);
    FR_Status = f_mount(NULL, "", 0);
    HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
  }

}