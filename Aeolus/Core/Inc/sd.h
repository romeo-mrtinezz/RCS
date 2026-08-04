#include "FreeRTOS.h"
#include "global.h"
#include <stdint.h>

void SD_Card_Write();
void SD_Card_Test();
void SD_Card_init();
void log_accel(uint8_t count, MessageQueue_t * buffer, osThreadId_t thread_id);
void log_pls(uint32_t time, AccData * accel_data);