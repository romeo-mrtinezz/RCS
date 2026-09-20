
/*
Simple 1DOF simulating pitch/yaw based on control signal from controller
Upgrade to 2DOF eventually
*/

#include "global.h"
#include <math.h>

#define FORCE           5    // N
#define MASS            5    // kg
#define ARM             0.5  // m
#define RADIUS_TUBE     0.0762 // m
#define LENGTH_TUBE     0.8  // m

float omega_prev = 0;

void dynamics(FullData *full_data, float dt, uint16_t D) {
    float thrust = D*FORCE;
    float alpha = ARM*thrust/I;
    float omega_new = omega_prev + alpha*dt;
    omega_prev = omega_new;
    // Calculate new pitch
    full_data->pitch = full_data->pitch + omega_new*dt + 0.5*alpha*pow(dt,2);
}
