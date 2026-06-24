"""
This file is aimed to develop a better understanding of EKF with examples
"""
import numpy as np 
import matplotlib.pyplot as plt
# import spicy
from scipy.spatial.transform import Rotation as R


# verifying libraries imported
a = np.array([[1, 2, 3],
              [4, 5, 6]])

print(a.shape)

# define simulation params
dt = 0.01 # polling IMU at 100Hz
T = 20    # seconds
N = int(T/dt)  # no. of samples (2000 in this case)

# create arrays
true_quat = np.empty(N)
estimated_quat = np.empty(N)
gyro_meas = np.empty((N,3))
accel_meas = np.empty((N,3))

# define ture motion of body
omega = [0.3, 0.2, 0.1] # rad/s
q_true = [1, 0, 0, 0]

for i in range(1,N):
    # integrate quaternion using omega
    # store q_true
    pass

# generate mock IMU data
gyro_bias = [0.01, -0.02, 0.015]
# gyro_meas = 
noise = np.random.normal(0,0.005, 3)

g = [0, 0, -9.81] # inertial frame
# rotate to body frame

#-------------------- EKF -------------------

x = [qw, qx, qy, qz]
q_est = [1, 0, 0, 0]

# plotting