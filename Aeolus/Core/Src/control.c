#include "control.h"

#if CONTROLLER == CTRL_PID
#include "pid.h"
static PID_params pitch, yaw;
static float pitch_u, yaw_u;
#elif CONTROLLER == CTRL_PID
#include "bangbang.h"
static BB_params pitch, yaw;
#else
#error "unknown CONTROLLER"
#endif

float comp_filter(float alpha, float dt, float prev_angle, float gyro_rate, float accel_angle)
{
    return alpha * (prev_angle + gyro_rate * dt) + (1 - alpha) * accel_angle;
}

void control_init(void)
{
#if CONTROLLER == CTRL_PID
    pid_init(&pitch);
    pid_init(&yaw);
#elif CONTROLLER == CTRL_BANGBANG
    bb_init(&pitch);
    bb_init(&yaw);
#endif
}

void control_update(float pitch_angle, float yaw_angle, float dt, control_out *out)
{
#if CONTROLLER == CTRL_PID
    pitch_u = pid_update(&pitch, 0, pitch_angle, dt);
    yaw_u   = pid_update(&yaw, 0, yaw_angle, dt);
    out->pitch_u = pitch_u;
    out->yaw_u   = yaw_u;
#elif CONTROLLER == CTRL_BANGBANG
    (void)dt;
    out->pitch_u = bb_update(&pitch, 0, pitch_angle) * BB_MAX_DUTY;
    out->yaw_u   = bb_update(&yaw, 0, yaw_angle) * BB_MAX_DUTY;
#endif
    out->pitch_error = pitch.error;
    out->yaw_error   = yaw.error;
}

void control_apply(void)
{
#if CONTROLLER == CTRL_PID
    select_thruster(&pitch, pitch_u, &yaw, yaw_u);
#elif CONTROLLER == CTRL_BANGBANG
    bb_select_thruster(&pitch, &yaw);
#endif
}