#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "ff.h"
static FATFS FatFs;
static FIL Fil;

// typedef struct {
//     float Kp, Ki, Kd;
//     float prev_error; 
//     float integral;
//     float min_duty, max_duty;
//     float error;
// } PID_params;

// Data types
typedef struct {
    float rate_x;
    float rate_y;
    float rate_z;
} GyroData;

typedef struct {
    float acc_x;
    float acc_y;
    float acc_z;
} AccData;

typedef struct {
    float est_pitch;
    float est_yaw;
} Attitude;

typedef struct {
    uint32_t timestamp; // ms
    float rate_x;
    float rate_y;
    float rate_z;
    float acc_x;
    float acc_y;
    float acc_z; 
    float pitch_accel;
    float yaw_accel; // add pitch/yaw gyro too if needed
    float pitch;
    float yaw;
} FullData;

typedef struct {
    uint32_t timestamp;
    float acc_x;
    float acc_y;
    float acc_z;
} MessageQueue_t;

// Functions
void accel_to_angle(const AccData accel_data, float *accel_pitch, float *accel_yaw);


#endif // __GLOBAL_H
