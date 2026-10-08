#include "bangbang.h"
#include "main.h"
#include <math.h>

#define DB_I 2.0f   // inside this, valves off
#define DB_O 4.0f   // between DB_I and DB_O, keep doing whatever we were doing

void bb_init(BB_params *bb)
{
    bb->state = 0;
    bb->error = 0;
    bb->command = 0;
}

int bb_update(BB_params *bb, float set_point, float angle_estimate)
{
    bb->error = set_point - angle_estimate;

    if (fabsf(bb->error) < DB_I)
        bb->command = 0;
    else if (fabsf(bb->error) < DB_O)
        bb->command = bb->state;
    else
        bb->command = (bb->error > 0) ? 1 : -1;   // same mapping as pid: +error -> ccr3 (pitch) / ccr2 (yaw)

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
    drive_axis(pitch->command, &TIM1->CCR3, &TIM1->CCR1);
    drive_axis(yaw->command, &TIM1->CCR2, &TIM1->CCR4);
    pitch->state = pitch->command;
    yaw->state = yaw->command;
}