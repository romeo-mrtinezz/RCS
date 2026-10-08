#ifndef __PID_H
#define __PID_H

// Struct with pointer good because we'll have 2 instances of PID (pitch yaw). actually, maybe not...
// also params have to survive between pid_update calls, e.g integral, prev error, needs to be global. 
typedef struct {
    float Kp, Ki, Kd;
    float prev_error; 
    float integral;
    float min_duty, max_duty;
    float error;
    float control_duty;
    float rate;          // error rate, deg/s (set in pid_update)
} PID_params;

void pid_init(PID_params *pid); 
// Returns the signed control value, -max_duty .. +max_duty
float pid_update(PID_params *pid, float set_point, float angle_estimate, float dt);
// pitch_u / yaw_u are the signed outputs of pid_update()
void select_thruster(const PID_params *pitch, float pitch_u,
                     const PID_params *yaw, float yaw_u);
#endif /*__PID_H*/