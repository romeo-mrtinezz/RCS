

#ifndef __PID_H
#define __PID_H

// Struct with pointer good because we'll have 2 instances of PID (pitch yaw). 
// also params have to survive between pid_update calls, e.g integral, prev error, needs to be global. 
typedef struct {
    float Kp, Ki, Kd;
    float prev_error; 
    float integral;
    float min_duty, max_duty;
    float error;
} PID_params;

float comp_filter(float alpha, int dt, float prev_angle, float gyro_rate, float accel_angle);
void pid_init(PID_params *pid); 
float pid_update(PID_params *pid, float set_point, float angle_estimate, float dt);
void select_thruster(float pitch_error, float pitch_duty, float yaw_error, float yaw_duty, float dt);
#endif /*__PID_H*/
