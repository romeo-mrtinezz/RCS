// #include "bangbang.h"
// #include "main.h"
// #include <math.h>

// #define DB_I 2.0f   // inside this, valves off
// #define DB_O 4.0f   // between DB_I  and DB_O, keep doing whatever we were doing

// void bb_init(BB_params *bb)
// {
//     bb->state = 0;
//     bb->error = 0;
//     bb->command = 0;
// }

// int bb_update(BB_params *bb, float set_point, float angle_estimate)
// {
//     bb->error = set_point - angle_estimate;

//     if (fabsf(bb->error) < DB_I)
//         bb->command = 0;
//     else if (fabsf(bb->error) < DB_O)
//         bb->command = bb->state;
//     else
//         bb->command = (bb->error > 0) ? 1 : -1;   // same mapping as pid: +error -> ccr3 (pitch) / ccr2 (yaw)

//     return bb->command;
// }

// // +1 -> pos valve full on, -1 -> neg valve full on, 0 -> both off
// static void drive_axis(int command, volatile uint32_t *pos_ccr, volatile uint32_t *neg_ccr)
// {
//     *pos_ccr = (command > 0) ? BB_MAX_DUTY : 0;
//     *neg_ccr = (command < 0) ? BB_MAX_DUTY : 0;
// }

// void bb_select_thruster(BB_params *pitch, BB_params *yaw)
// {
//     drive_axis(pitch->command, &TIM1->CCR1, &TIM1->CCR2);
//     drive_axis(yaw->command, &TIM1->CCR3, &TIM1->CCR4);
//     pitch->state = pitch->command;
//     yaw->state = yaw->command;
// }







////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////




#include "bangbang.h"
#include "main.h"
#include <math.h>
 
#define DB_I        0.5f    // inside this, valves off
#define DB_O        2.0f    // between DB_I and DB_O, hysteresis zone
#define RL1         6.0f   // rate limit in deadband deg/s
#define RL2         12.0f  // rate limit outside deadband deg/s

void bb_init(BB_params *bb)
{
    bb->state = 0;
    bb->error = 0;
    bb->command = 0;

       // apply ccr changes immediately instead of at the next 100 ms pwm period
   TIM1->CCMR1 &= ~(TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE);
   TIM1->CCMR2 &= ~(TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE);

}

int bb_update(BB_params *bb, float set_point, float angle_estimate, float angular_rate)
{
    float error = set_point - angle_estimate;
    bb->error = error;
 
    // inside the outer band and moving fast
    if (fabs(angular_rate) > RL1 && fabs(bb->error) < DB_O)
    {
        if (bb->error * angular_rate > 0)   // heading towards the target: brake
        {
            bb->command = (angular_rate > 0) ? -1 : 1;
        }
        else                                // heading away from the target: push it back
        {
            bb->command = (bb->error > 0) ? 1 : -1;
        }
    }
    else if (fabs(bb->error) < DB_I) // if error < inner deadband
    {
        bb->command = 0; // turn thruster off
    }
    else if (fabs(bb->error) < DB_O) // if larger than inner db but less than outer db
    {
        bb->command = bb->state; // otherwise, leave command as current state (don't turn off or on)
    }
    else if (bb->error * angular_rate > 0 && fabs(angular_rate) > RL2)
    {
        // outside the band, heading towards the target too fast: brake
        bb->command = (angular_rate > 0) ? -1 : 1;
    }
    else
    {
        // outside the band: push towards the target
        bb->command = (bb->error > 0) ? 1 : -1;
    }

    return bb->command;
}
 
// +1 -> pos valve full on, -1 -> neg valve full on, 0 -> both off
static void drive_axis(int command, volatile uint32_t *pos_ccr, volatile uint32_t *neg_ccr)
{
    *pos_ccr = (command > 0) ? BB_MAX_DUTY : 0;
    *neg_ccr = (command < 0) ? BB_MAX_DUTY : 0;
}
 
void bb_select_thruster(BB_params *pitch, BB_params *yaw)
{
    drive_axis(pitch->command, &TIM1->CCR1, &TIM1->CCR2);   // same mapping as pid.c
    drive_axis(yaw->command, &TIM1->CCR3, &TIM1->CCR4);
    pitch->state = pitch->command;
    yaw->state = yaw->command;
}
 






/////////////////////////////////////////////////
/////////////////////////////////////////////////
















// #include "bangbang.h"
// #include "main.h"
// #include <math.h>

// #define LOOKAHEAD  0.45f   // s, one control step
// #define DB         1.0f   // deg, valves off inside this

// void bb_init(BB_params *bb)
// {
//     bb->state = 0;
//     bb->error = 0;
//     bb->command = 0;

//     // apply valve changes immediately, not at the next pwm period
//     TIM1->CCMR1 &= ~(TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE);
//     TIM1->CCMR2 &= ~(TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE);
// }

// int bb_update(BB_params *bb, float set_point, float angle_estimate, float angular_rate)
// {
//     bb->error = set_point - angle_estimate;

//     // predicted error one step ahead
//     float next_error = bb->error - angular_rate * LOOKAHEAD;

//     if (next_error > DB)
//         bb->command = 1;      // will fall short: push towards target
//     else if (next_error < -DB)
//         bb->command = -1;     // will overshoot: push the other way
//     else
//         bb->command = 0;      // close enough: off

//     return bb->command;
// }

// static void drive_axis(int command, volatile uint32_t *pos_ccr, volatile uint32_t *neg_ccr)
// {
//     *pos_ccr = (command > 0) ? BB_MAX_DUTY : 0;
//     *neg_ccr = (command < 0) ? BB_MAX_DUTY : 0;
// }

// void bb_select_thruster(BB_params *pitch, BB_params *yaw)
// {
//     drive_axis(pitch->command, &TIM1->CCR1, &TIM1->CCR2);
//     drive_axis(yaw->command, &TIM1->CCR3, &TIM1->CCR4);
//     pitch->state = pitch->command;
//     yaw->state = yaw->command;
// }