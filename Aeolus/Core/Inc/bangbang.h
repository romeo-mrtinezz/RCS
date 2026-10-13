#ifndef __BANGBANG_H
#define __BANGBANG_H

#define BB_MAX_DUTY 9999

typedef struct {
    int state;
    float error;
    int command;
} BB_params;

void bb_init(BB_params *bb);
int  bb_update(BB_params *bb, float set_point, float angle_estimate, float angular_rate);
void bb_select_thruster(BB_params *pitch, BB_params *yaw);

#endif /*__BANGBANG_H*/