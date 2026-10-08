#ifndef __BANGBANG_H
#define __BANGBANG_H

#define BB_MAX_DUTY 9999

typedef struct {
    int state;      // what the valves are doing now: -1, 0, +1
    float error;
    int command;    // what we want: -1, 0, +1
} BB_params;

void bb_init(BB_params *bb);
int  bb_update(BB_params *bb, float set_point, float angle_estimate);
void bb_select_thruster(BB_params *pitch, BB_params *yaw);

#endif /*__BANGBANG_H*/