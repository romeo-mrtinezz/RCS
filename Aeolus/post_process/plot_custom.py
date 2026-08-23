"""
Parses csv file containing full data, either from live telemetry or sd card,
and plots each column against time
- 14 columns including timestamp
- Assume no headers in csv file

Change desired list before running
"""

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# Enter file name to parse
# file = input("Type filename in the form <example.csv>\n")
df = pd.read_csv(r"C:\Users\romeo\OneDrive\Desktop\RCS\Aeolus\post_process\test.csv", header=None, delimiter=",", dtype=np.float32)

columns = {
    "Timestamp"    : df.iloc[:,0],
    "Rate_x"       : df.iloc[:,1],
    "Rate_y"       : df.iloc[:,2],
    "Rate_z"       : df.iloc[:,3],
    "Acc_x"        : df.iloc[:,4],
    "Acc_y"        : df.iloc[:,5],
    "Acc_z"        : df.iloc[:,6],
    "Pitch_accel"  : df.iloc[:,7],
    "Yaw_accel"    : df.iloc[:,8],
    "Pitch"        : df.iloc[:,9],
    "Yaw"          : df.iloc[:,10],
    "Pitch_error"  : df.iloc[:,11],
    "Yaw_error"    : df.iloc[:,12],
    "Control_duty" : df.iloc[:,13]
}

desired = ["Rate_x","Rate_y"]

# Plot on same axes
def plot_multiple(timestamp, columns: dict, desired: list[str]):
    fig, ax = plt.subplots()

    for variable in desired: # check if variable is a dictionary key
        if variable in columns:
            ax.plot(timestamp, columns[variable], label=variable)

    ax.set_xlabel("Time (ms)")
    ax.grid()
    ax.legend()
    plt.show()

def main():
    # plot_one(timestamp, pitch, "Pitch")
    plot_multiple(columns["Timestamp"], columns, desired)

if __name__ == "__main__":
    main()
    