/* pseudocode

*/ 

void gyro_callback() {
    // read 6 bytes and append to buffer (replace old data?)
    // is called every 1ms (1kHz)
};

void acc_callback() {

};

void comp_filter() {
/*
inputs: pointer to gyro_data and accel_data, timestamp?
outputs: none

*/
};

void run_pid_alg() {
    

};

void comp_filter(alpha, prev_pitch) {
/* 
outputs filtered pitch and yaw by fusing gyro and accel data
accel to angle
*/
   pitch = alpha*(prev_pitch + gyro_x * dt) + (1-alpha)*acc_x; 
   yaw =  alpha*(prev_yaw + gyro_y * dt) + (1-alpha)*acc_y; 

};

void log_to_sd() {
    // with DMA so cpu won't be blocked, and can do other tasks
    // and ISR
};

int main() {
    // dt is the period of your sampling freq f, so dt = 1/f. 
    // we integrating, over 1ms, there was 2deg/s, so multiply
    dt = 

    while (1) {
        read_data();
        comp_filter(gyro_data, accel_data, timestamp?);
        run_pid_alg();
        pwm_logic();
        log_to_sd();
    }

}
