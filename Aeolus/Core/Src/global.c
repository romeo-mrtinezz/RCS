
#include "global.h"
#include "math.h"

// Pitch is defined as rotation about the 

// const indicates to the compiler that the variable is read only, and will throw an error if try to modify
void accel_to_angle(const AccData accel_data, float *accel_pitch, float *accel_yaw) {
  *accel_pitch = -(float)atan2(-accel_data.acc_z, -accel_data.acc_x); // shouldn't matter if in mg z,x
  *accel_yaw = (float)atan2(-accel_data.acc_y, -accel_data.acc_x); // radians                     y,x
  
  *accel_pitch = *accel_pitch * 180.0f/M_PI; // degrees
  *accel_yaw = *accel_yaw * 180.0f/M_PI; 
}

// implemen dead reckoning, just comp filter but with alpha = 1 

// debug functions that print stuff, or actuate stuff, or test sequences
// char usb_buf[100];
// sprintf(usb_buf, "%lu,%.2f,%.2f,%.2f\n", msg.timestamp, gyro_data.rate_x, gyro_data.rate_y, gyro_data.rate_z);
