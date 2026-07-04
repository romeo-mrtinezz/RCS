/* pseudocode

*/ 

#include "bmi088.h"
#include "stm32g4xx_hal_def.h"
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

typedef struct {
  uint32_t timestamp;
  float acc_x;
  float acc_y;
  float acc_z;
} MessageQueue_t;

AccData accel_data;

void StartReadIMU(void *argument)
{ 
  /* USER CODE BEGIN StartReadIMU */
    MessageQueue_t msg;
  /* Infinite loop */
  for(;;)
  {
    accel_burst_read(&accel_data);
    msg.timestamp = xTaskGetTickCount() // uint32? 
    msg.acc_x = accel_data->acc_x
    msg.acc_y = accel_data->acc_y
    msg.acc_z = accel_data->acc_z
    osMessageQueuePut(messageQueueHandle, &msg, 0, 0);
    osDelay(100); // 10Hz
  }
  /* USER CODE END StartReadIMU */
}

void StartLog(void *argument)
{
  /* USER CODE BEGIN StartLog */
  MessageQueue_t msg;
  /* Infinite loop */
  for(;;)
  {
    // 0 timeout means will return immediately - I want this because I want to maintain the 1Hz
    // There are 12 elements in buffer, so there should always be 10 in there anyways since I'm reading at 10Hz
    osMessageQueueGet(messageQueueHandle, NULL, 0, 0);
    log()
    osDelay(1000); // 1Hz, every second
  }
  /* USER CODE END StartLog */
}

// freertos init
messageQueueHandle = osMessageQueueNew (12, sizeof(MessageQueue_t), &messageQueue_attributes);
readIMUHandle = osThreadNew(StartReadIMU, NULL, &readIMU_attributes);
logHandle = osThreadNew(StartLog, NULL, &log_attributes);
