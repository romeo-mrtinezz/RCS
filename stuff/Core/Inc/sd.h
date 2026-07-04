#include "FreeRTOS.h"
#include <stdint.h>

void SD_Card_Write();
void SD_Card_Test();
void SD_Card_init();
void log_accel(uint8_t count, MessageQueue_t * buffer, osThreadId_t thread_id);

