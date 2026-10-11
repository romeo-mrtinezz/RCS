#ifndef CONFIG_H
#define CONFIG_H

// Feel free to change these params

// PID --------------------------- used in pid.c
// Currently behaving like bang bang
#define KP                    = 10000
#define KI                    = 0
#define KD                    = 0
#define MAX_DUTY              = 10000 // Divide by 100 for duty cycle percentage
#define MIN_DUty              = 0
#define ANGLE_ERROR_TOLERANCE = 2 // Degrees from vertical

// FreeRTOS Task Rates in Hz
#define STREAM_RATE           = 10
#define LOG_RATE              = 170   
#define CONTROL_ALG_RATE      = 10 
#define READ_IMU_RATE         = 100

/*
If thrusters are in wrong axes, go to select_thrusters function in pid.c
swap every CCR1 with CCR2, and every CCR3 with CCR4
*/




#endif /* __CONFIG_H */