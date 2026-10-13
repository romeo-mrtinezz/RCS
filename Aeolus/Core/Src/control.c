#include "control.h"

#if CONTROLLER == CTRL_PID
#include "pid.h"
static PID_params pitch, yaw;
static float pitch_u, yaw_u;
#elif CONTROLLER == CTRL_BANGBANG
#include "bangbang.h"
static BB_params pitch, yaw;
#else
#error "unknown CONTROLLER"
#endif

#define SP_MAX 15.0f   // largest setpoint allowed, deg
static volatile float sp_pitch = 0, sp_yaw = 0;

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

void control_update(float pitch_angle, float yaw_angle, float pitch_rate, float yaw_rate,
                    float dt, control_out *out)
{
#if CONTROLLER == CTRL_PID
    (void)pitch_rate; (void)yaw_rate;   // pid works out its own rate
    pitch_u = pid_update(&pitch, sp_pitch, pitch_angle, dt);
    yaw_u   = pid_update(&yaw, sp_yaw, yaw_angle, dt);
    out->pitch_u = pitch_u;
    out->yaw_u   = yaw_u;
#elif CONTROLLER == CTRL_BANGBANG
    (void)dt;
    out->pitch_u = bb_update(&pitch, sp_pitch, pitch_angle, pitch_rate);
    out->yaw_u   = bb_update(&yaw, sp_yaw, yaw_angle, yaw_rate);
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

void control_reset(void)
{
#if CONTROLLER == CTRL_PID
    pitch.integral = 0;
    yaw.integral   = 0;
#endif
}

static float clampf(float v, float lim)
{
    return v > lim ? lim : (v < -lim ? -lim : v);
}

void control_set_target(float pitch_deg, float yaw_deg)
{
    sp_pitch = clampf(pitch_deg, SP_MAX);
    sp_yaw   = clampf(yaw_deg, SP_MAX);
}