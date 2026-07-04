
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
    float pitch;
    float yaw;
    int dt;
} Attitude;

typedef struct {
    uint32_t timestamp;
    float acc_x;
    float acc_y;
    float acc_z;
} MessageQueue_t;
