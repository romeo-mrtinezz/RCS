

#ifndef __PID_H
#define __PID_H

// Struct with pointer good because we'll have 2 instances of PID (pitch yaw). 
// also params have to survive between pid_update calls, e.g integral, prev error, needs to be global. 
typedef struct {
    float Kp, Ki, Kd;
    float prev_error; 
    float integral;
    float min_duty, max_duty;
} PID_params;

int comp_filter(float alpha, int dt, int prev_pitch, float gyro_rate, float accel_angle);
void pid_init(PID_params *pid); 
float pid_update(PID_params *pid, float set_point, float angle_estimate, float dt);
void select_thruster(float pitch_error, float pitch_duty)
#endif /*__PID_H*/
