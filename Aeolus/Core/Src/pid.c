

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
// extern TIM_HandleTypeDef htim1;



// Just 1 axis for now
float comp_filter(float alpha, float dt, float prev_angle, float gyro_rate, float accel_angle) {
    float angle = alpha*(prev_angle + gyro_rate * dt) + (1-alpha)*accel_angle;
    return angle;
}

// pass in pointer to pid so we can update them directly instead of making 100 copies
void pid_init(PID_params *pid) {
    pid->Kp = 80, pid->Ki = 2, pid->Kd = 2;
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
    pid->prev_error = error;

    /*The following should produce the same magnitude of control signal, the select thruster logic is a separate function
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

    float control = abs(P+I+D); // absolute value because we can't have -ve duty cycle

    // Saturation and anti-windup back calculation to remove error 
    // accumulated in this timestep
    if (control > pid->max_duty) {
        control = pid->max_duty;
        pid->integral -= error*dt; // can put a gain kt on this too if need be
    }
    else if (control < pid->min_duty) {
        control = pid->min_duty;
        pid->integral -= error*dt;
    }
    
    return (uint16_t)control; // CCR has to be uint16_t
}   

void select_thruster(float pitch_error, float pitch_duty, float yaw_error, float yaw_duty, float dt) {
    // map PWM to total impulse desired? thrust*seconds. maybe not needed tbh.
    int ACCEPTABLE = 5; // degrees
    if (pitch_error < -ACCEPTABLE) {
        // actuate pitch thruster in +ve
        TIM1->CCR1 = pitch_duty; 
        TIM1->CCR2 = 0; // make sure only 1 actuated in the same axis
    }
    else if (pitch_error > ACCEPTABLE ) {
        // actuate pitch thruster in -ve direction
        TIM1->CCR1 = 0;
        TIM1->CCR2 = pitch_duty;
    }
    else {
        // do not actuate pitch thruster (default)
        TIM1->CCR1 = 0;
        TIM1->CCR2 = 0;        
    }

    if (yaw_error < -ACCEPTABLE) {
        // actuate yaw thruster in +ve
        TIM1->CCR3 = yaw_duty;
        TIM1->CCR4 = 0; 
    }
    else if (yaw_error > ACCEPTABLE ) {
        // actuate yaw thruster in -ve direction
        TIM1->CCR3 = 0;
        TIM1->CCR4 = yaw_duty; 
    }
    else {
        // do not actuate yaw thruster
        TIM1->CCR3 = 0;
        TIM1->CCR4 = 0; 
    }


}
