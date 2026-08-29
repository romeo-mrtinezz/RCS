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
df = pd.read_csv(r"C:\Users\romeo\OneDrive\Desktop\RCS\Aeolus\post_process\COM3_2026_08_24.21.35.45.772.csv", header=None, delimiter=",", dtype=np.float32)

columns = {
    "Timestamp"    : df.iloc[:,0]/1000, # s
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
    "Pitch_duty"   : df.iloc[:,13]/100, # %
    "Yaw_duty"     : df.iloc[:,14]
}

desired1 = ["Pitch", "Pitch_accel"]
desired_left = ["Pitch"]
desired_right = ["Pitch_duty"]
# desired2 = ["Pitch_duty"]

# Plot on same axes
def plot_multiple(timestamp, columns: dict, desired: list[str], title = None, ylabel = None):
    fig, ax = plt.subplots()

    for variable in desired: # check if variable is a dictionary key
        if variable in columns:
            ax.plot(timestamp, columns[variable], label=variable)

    ax.set_xlabel("Time (s)")
    if isinstance(title, str):
        ax.set_title(title)
    if isinstance(ylabel, str):
        ax.set_ylabel(ylabel)

    ax.minorticks_on()
    ax.grid(which="minor")
    ax.legend()

# Plot multiple but with different units on same axes
def plot_different(timestamp, columns: dict, desired_left: list[str], desired_right: list[str],
                    title=None, ylabel_left=None, ylabel_right=None):
    fig, ax = plt.subplots()
    ax_twin = ax.twinx()
    for variable in desired_left:
        if variable in columns:
            ax.plot(timestamp, columns[variable], label=variable, color="blue")

    for variable in desired_right:
        if variable in columns:
            ax_twin.plot(timestamp, columns[variable], label=variable, color="green")

    if "Pitch_duty" in desired_right or "Yaw_duty" in desired_left:
        ax_twin.axhline(32,color='r')
        ax_twin.axhline(68, color='r')
    ax.axhline(0, color='black',linestyle='dashed')
    ax.set_xlabel("Time (s)")
    if isinstance(title, str):
        ax.set_title(title)
    if isinstance(ylabel_left, str):
        ax.set_ylabel(ylabel_left)
    if isinstance(ylabel_right, str):
        ax_twin.set_ylabel(ylabel_right)

    ax.minorticks_on()
    ax.grid(which="major", linewidth=0.8)
    ax.grid(which="minor", linewidth=0.4, alpha=0.5)

    # combine legends from both axes
    lines1, labels1 = ax.get_legend_handles_labels()
    lines2, labels2 = ax_twin.get_legend_handles_labels()
    ax.legend(lines1 + lines2, labels1 + labels2)


def main():
    # plot_one(timestamp, pitch, "Pitch")
    plot_multiple(columns["Timestamp"], columns, desired1)
    plot_different(columns["Timestamp"], columns, desired_left, desired_right, ylabel_left="Degrees", ylabel_right="Percentage")
    # plot_multiple(columns["Timestamp"], columns, desired2)

    plt.show()

if __name__ == "__main__":
    main()
