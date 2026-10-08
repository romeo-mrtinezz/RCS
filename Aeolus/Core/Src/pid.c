// Attempt at implementing PID control algorithm

// send control signal to pwm logic function. if control > x, duty cycle = y etc.

// output = accel reading

// Saturation, to prevent controller from trying to drive a higher output than is physically possible (e.g. 98% duty cycle)
// if control > 78, then max is 89% duty cycle for example. 

// Anti-wind up by removing the error accumulated over the timestep where the control signal is saturated. 
// without it we would have a large over shoot and large delay in converging back to setpoint, aka large settling time

/* pid to pwm logic?
I tested 15Hz (66ms period) at 50% duty cycle and I hear clicking, which means it can turn on and off in that 66ms period.
So actuation time of 33ms. at 20Hz I couldnt hear a difference, but that would make it 50ms period, so 25ms actuation time
Assuming 25ms actuation time, we run at 15Hz, then our duty cycle is limited from 38% to 62% which isn't a lot.
Cal could get from 13-87% because he ran at a lower frequency. I almost feel like a lower thrust output would make this project better,
because it would for the varying duty cycles to take effect


*/

#include "PID.h"
#include "main.h"
#include "global.h"
#include <stdlib.h>
#include <math.h>
// extern TIM_HandleTypeDef htim1;



// pass in pointer to pid so we can update them directly instead of making 100 copies
void pid_init(PID_params *pid) {
    pid->Kp = 600, pid->Ki = 0, pid->Kd = 1200;    // was 1200 with dt 0.2, same behaviour now dt is 0.1
    pid -> integral = 0;
    pid -> prev_error = 0; 
    pid->min_duty = 0, pid->max_duty = 10000; // Duty cycle max 3800 6200
}

float pid_update(PID_params *pid, float set_point, float angle_estimate, float dt) {
    float error = set_point - angle_estimate;
    pid->error = error;
    pid->integral += error*dt;

    float P = pid->Kp*error;
    float I = pid->Ki*pid->integral;
    float D = pid->Kd*(error-pid->prev_error)/dt;
    pid->rate = (error - pid->prev_error) / dt;   // error rate, deg/s
    pid->prev_error = error;

    /*The following should produce the same magnitude of control signal, the select thruster logic is a separate function
    for kp = 80, ki = 2, kd = 2
    
    LOOP 1
    error  = -20 deg
    integral = -2
    P = -1600
    I = -4
    D = -200
    prev_error = -20 deg
    control = -1804

    LOOP 2
    error = -10 deg
    integral = -3
    P = -800
    I = -6
    D = 100
    prev_error = -10
    control = -706

    LOOP 1
    error = 20 deg
    integral = 2
    P = 1600
    I = 4
    D = 200
    prev_error = 20 deg
    control = 1804

    LOOP 2
    error = 10 deg
    integral = 3
    P = 800
    I = 6
    D = -100
    prev_error = 10
    control = 706
    */

    // Signed control output. Positive = fire the thruster that corrects a
    // positive error, negative = fire the opposite thruster. Because D is
    // included, a fast swing back toward 0 can flip the sign and fire the
    // opposite thruster to brake, even while the error is still positive.
    float u = P + I + D;

    // Saturation (both directions) + anti-windup back-calculation
    if (u > pid->max_duty) {
        u = pid->max_duty;
        pid->integral -= error*dt;
    }
    else if (u < -pid->max_duty) {
        u = -pid->max_duty;
        pid->integral -= error*dt;
    }

    pid->control_duty = u;
    return u;   // signed, -max_duty .. +max_duty
}

// ============================ DEADBAND SETTINGS ============================
// Pick ONE deadband mode by changing DEADBAND_MODE below.
//
// DEADBAND_ON_ERROR  : no thruster fires while |angle error| < DEADBAND_DEG.
//                      Simple: "nothing happens within X deg of level".
//                      Inside the band the D term cannot brake.
//
// DEADBAND_ON_OUTPUT : no thruster fires while |P+I+D| < Kp * DEADBAND_DEG.
//                      Same as above when the board is still, but the D term
//                      can still fire a braking pulse near level if the board
//                      is rotating fast.
//
// In both modes the valve is chosen by the SIGN of P+I+D, so D can fire the
// opposite thruster to slow a swing.
#define DEADBAND_ON_ERROR   0
#define DEADBAND_ON_OUTPUT  1

#define DEADBAND_MODE  DEADBAND_ON_OUTPUT  // <-- change this
#define DEADBAND_DEG   0.5f                 // <-- deadband size in degrees
#define NEAR_DEG       0.5f                 // within this angle...
#define RATE_DEADBAND  8.0f                 // ...and slower than this (deg/s), don't fire
// ==========================================================================

// Fire one valve of an axis from a signed control value.
//   u > 0 -> pos_ccr opens (corrects positive error)
//   u < 0 -> neg_ccr opens (corrects negative error)
static void drive_axis(const PID_params *pid, float u,
                       volatile uint32_t *pos_ccr, volatile uint32_t *neg_ccr) {
    float mag = fabsf(u);

#if DEADBAND_MODE == DEADBAND_ON_ERROR
    int inside_deadband = fabsf(pid->error) < DEADBAND_DEG;
#else
    int inside_deadband = mag < pid->Kp * DEADBAND_DEG;
#endif

    // close to level and moving slowly: leave it to gravity, stops small drifts triggering big kicks
    if (fabsf(pid->error) < NEAR_DEG && fabsf(pid->rate) < RATE_DEADBAND) inside_deadband = 1;

    if (inside_deadband || mag == 0.0f) {         // both valves shut
        *pos_ccr = 0;
        *neg_ccr = 0;
        return;
    }
    if (mag < pid->min_duty) mag = pid->min_duty; // floor so the valve actually opens
    if (mag > pid->max_duty) mag = pid->max_duty;
    uint32_t duty = (uint32_t)mag;
    if (u > 0) { *neg_ccr = 0; *pos_ccr = duty; } // close the other valve first
    else       { *pos_ccr = 0; *neg_ccr = duty; }
}

// Valve mapping:
//   Pitch: +u -> CCR3, -u -> CCR1
//   Yaw:   +u -> CCR2, -u -> CCR4
void select_thruster(const PID_params *pitch, float pitch_u,
                     const PID_params *yaw, float yaw_u) {
    drive_axis(pitch, pitch_u, &TIM1->CCR1, &TIM1->CCR3);
    drive_axis(yaw,   yaw_u,   &TIM1->CCR2, &TIM1->CCR4);
}   