
#include "global.h"
#include "math.h"

// const indicates to the compiler that the variable is read only, and will throw an error if try to modify
void accel_to_angle(const AccData accel_data, float *accel_pitch, float *accel_yaw) {
  *accel_pitch = (float)atan2(accel_data.acc_z, accel_data.acc_x); // shouldn't matter if in mg
  *accel_yaw = (float)atan2(accel_data.acc_y, accel_data.acc_x); // radians
  
  *accel_pitch = *accel_pitch * 180.0f/M_PI; // degrees
  *accel_yaw = *accel_yaw * 180.0f/M_PI; 
};