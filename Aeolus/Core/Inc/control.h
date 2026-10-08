#ifndef __CONTROL_H
#define __CONTROL_H

// pick the controller here, everything else calls control_*
#define CTRL_PID       0
#define CTRL_BANGBANG  1
#define CONTROLLER     CTRL_PID

typedef struct {
    float pitch_error, yaw_error;   // deg
    float pitch_u, yaw_u;           // signed command, -10000..10000 (logged as pitch_duty / yaw_duty)
} control_out;

float comp_filter(float alpha, float dt, float prev_angle, float gyro_rate, float accel_angle);
void  control_init(void);
void  control_update(float pitch, float yaw, float dt, control_out *out);  // compute only
void  control_apply(void);                                                 // write the valves from the last update
void  control_reset(void);                                                 // zero the integral (call while disarmed)

#endif /*__CONTROL_H*/