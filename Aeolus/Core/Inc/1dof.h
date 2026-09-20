/*
Simple 1DOF simulating pitch/yaw based on control signal from controller
Upgrade to 2DOF eventually
*/
#ifndef __1DOF_H__
#define __1DOF_H__

void dynamics(FullData *full_data, float dt, uint16_t D);

#endif /* __1DOF_H__ */
