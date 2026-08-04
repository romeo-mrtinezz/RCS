/* pseudocode

*/ 

#include "bmi088.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "stm32g4xx_hal_def.h"
#include "stm32g4xx_hal_gpio.h"
#include <stdint.h>
void gyro_callback() {
    // read 6 bytes and append to buffer (replace old data?)
    // is called every 1ms (1kHz)
};

void acc_callback() {

};

void comp_filter() {
/*
inputs: pointer to gyro_data and accel_data, timestamp?
outputs: none

*/
};

void run_pid_alg() {
    

};

void comp_filter(alpha, prev_pitch) {
/* 
outputs filtered pitch and yaw by fusing gyro and accel data
accel to angle
*/
   pitch = alpha*(prev_pitch + gyro_x * dt) + (1-alpha)*acc_x; 
   yaw =  alpha*(prev_yaw + gyro_y * dt) + (1-alpha)*acc_y; 

};

void log_to_sd() {
    // with DMA so cpu won't be blocked, and can do other tasks
    // and ISR
    
};

int main() {
    // dt is the period of your sampling freq f, so dt = 1/f. 
    // we integrating, over 1ms, there was 2deg/s, so multiply
    dt = 

    while (1) {
        read_data();
        comp_filter(gyro_data, accel_data, timestamp?);
        run_pid_alg();
        pwm_logic();
        log_to_sd();
    }

}

logging logic
/*
read accel every 100ms, add to buffer
buffer = [1,2,3,4,5,6,7,8,9,10]
write to csv every 1000ms or 1second (so we write 10 things at a time). 
Make sure buffer is protected by a mutex. I might see jumps between buffers then potensh, eh.

OR

read accel every 10ms, add to buffer
buffer = [1,2,3,4,5,6,7,8,9,10]
every second read the buffer and write whats there (FIFO)
*/

// #include "sd.h"
// #include "main.h"
// #include "FreeRTOS.h"
// #include "FreeRTOSConfig.h"
// #include "global.h"
// #include "ff.h"
// #include <string.h>
// #include <stdio.h>
// extern osMessageQueueId_t messageQueueHandle;
// static FATFS FatFs;
// static FIL Fil;


// typedef struct {
//   uint32_t timestamp;
//   float acc_x;
//   float acc_y;
//   float acc_z;
// } MessageQueue_t;

// AccData accel_data;

// void StartReadIMU(void *argument)
// { 
//   /* USER CODE BEGIN StartReadIMU */
//     MessageQueue_t msg;
//   /* Infinite loop */
//   for(;;)
//   {
//     accel_burst_read(&accel_data);
//     msg.timestamp = xTaskGetTickCount(); // uint32? 
//     msg.acc_x = accel_data.acc_x;
//     msg.acc_y = accel_data.acc_y;
//     msg.acc_z = accel_data.acc_z;
//     osMessageQueuePut(messageQueueHandle, &msg, 0, 0);
//     osDelay(100); // 10Hz
//   }
//   /* USER CODE END StartReadIMU */
// }

// void StartLog(void *argument)
// {
//   /* USER CODE BEGIN StartLog */
//   MessageQueue_t buffer[12];
//   SD_Card_init();
//   /* Infinite loop */
//   for(;;)
//   {
//     // 0 timeout means will return immediately - I want this because I want to maintain the 1Hz
//     // There are 12 elements in buffer, so there should always be 10 in there anyways since I'm reading at 10Hz
//     // osMessageQueueGet(messageQueueHandle, NULL, 0, 0); // this only grabs 1 element, not all 10
//     uint8_t count = 0;
//     while (count < 10 && osMessageQueueGet(messageQueueHandle, &buffer[count], NULL, 0) == osOK) {
//         count++;
//     }

//     log_accel(count, buffer, osThreadGetId());
//     osDelay(1000); // 1Hz, every second
//   }
//   /* USER CODE END StartLog */
// }

// // freertos init
// messageQueueHandle = osMessageQueueNew (12, sizeof(MessageQueue_t), &messageQueue_attributes);
// readIMUHandle = osThreadNew(StartReadIMU, NULL, &readIMU_attributes);
// logHandle = osThreadNew(StartLog, NULL, &log_attributes);

// void SD_Card_init() {
//   FRESULT FR_Status;
//   FATFS *FS_Ptr;
//   UINT RWC, WWC; // Read/Write Word Counter
//   DWORD FreeClusters;
//   uint32_t TotalSize, FreeSpace;
//   char TxBuffer[250]; 
  
//   do {
//     // Mount SD card
//     FR_Status = f_mount(&FatFs, "", 0);
//     if (FR_Status != FR_OK)
//     {
//         sprintf(TxBuffer, "Error! While Mounting SD Card, Error Code: (%i)\r\n", FR_Status);
//         break;
//     }
//     sprintf(TxBuffer, "SD Card Mounted Successfully! \r\n\n");

//     // Get SD card size and free space
//     f_getfree("", &FreeClusters, &FS_Ptr);
//     TotalSize = (uint32_t)((FS_Ptr->n_fatent - 2) * FS_Ptr->csize * 0.5);
//     FreeSpace = (uint32_t)(FreeClusters * FS_Ptr->csize * 0.5);

//     // Open csv file, if doesn't exist, creates it
//     FR_Status = f_open(&Fil, "accel_data.csv", FA_WRITE | FA_READ | FA_OPEN_ALWAYS);
//     if(FR_Status != FR_OK)
//     {
//         sprintf(TxBuffer, "Error! While Creating/Opening A New Text File, Error Code: (%i)\r\n", FR_Status);
//         break;
//     }

//     // Write headers
//     f_puts("time_ms,acc_x,acc_y,acc_z\n", &Fil); //\n move cursor to front, new line
//     // f_lseek(&Fil, f_size(&Fil)) // look for end of file
//     f_sync(&Fil);
//     } while(0);
// }


// void log_accel(uint8_t count, MessageQueue_t * buffer, osThreadId_t thread_id) {
//   FRESULT FR_Status;
//   FATFS *FS_Ptr;
//   UINT RWC, WWC; // Read/Write Word Counter
//   char TxBuffer[250]; 
//   char RW_Buffer[200];

// do {
//     // Write 10 elements to sd card
//     for (uint8_t i = 0; i < count; i++) {
//       sprintf(RW_Buffer, "%lu,%.2f,%.2f,%.2f\n",
//               buffer[i].timestamp, buffer[i].acc_x, buffer[i].acc_y, buffer[i].acc_z);
//       f_write(&Fil, RW_Buffer, strlen(RW_Buffer), &WWC); 
//       }

//     f_sync(&Fil);
    

//     if (xTaskGetTickCount() > pdMS_TO_TICKS(20000)) { // after 20s dismount sdcard and end task
//       // Unmount SD card
//       f_close(&Fil);
//       FR_Status = f_mount(NULL, "", 0);
//       HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
//       osThreadTerminate(thread_id);
//     }
//   } while(0);
// }