"""
plot_accel.py

DISCLAIMER, THIS IS AI GENERATED
use for sd card data

Reads a CSV file containing accelerometer data (time_ms, acc_x, acc_y, acc_z)
and produces:
  - 3 separate 2D line graphs: acc_x vs time, acc_y vs time, acc_z vs time
  - 1 combined 3D trajectory plot of (acc_x, acc_y, acc_z)

Usage:
    python plot_accel.py data.csv
"""
# py  post_process\plot_accel.py accel_data.csv (in terminal to run)

import os
import sys
import pandas as pd
import matplotlib.pyplot as plt

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
# "data" sits one level up from this script (project root), alongside Core/ and cmake/
OUTPUT_DIR = os.path.join(SCRIPT_DIR, "..", "data")


def get_next_run_prefix(output_dir: str) -> str:
    """
    Looks in output_dir for existing folders/files named accel_1, accel_2, ...
    and returns the next unused prefix, e.g. 'accel_3'.
    """
    if not os.path.isdir(output_dir):
        return "accel_1"

    existing_nums = []
    for name in os.listdir(output_dir):
        if name.startswith("accel_"):
            num_part = name.split("_")[1].split(".")[0]
            if num_part.isdigit():
                existing_nums.append(int(num_part))

    next_num = max(existing_nums, default=0) + 1
    return f"accel_{next_num}"


def load_data(filepath: str) -> pd.DataFrame:
    # sep=None + engine="python" auto-detects whether the file is
    # comma-separated, tab-separated, or whitespace-separated.
    df = pd.read_csv(filepath, sep=None, engine="python")
    df.columns = [c.strip() for c in df.columns]

    required = {"time_ms", "acc_x", "acc_y", "acc_z"}
    missing = required - set(df.columns)
    if missing:
        raise ValueError(f"CSV is missing required columns: {missing}")

    return df


def plot_axis_vs_time(df: pd.DataFrame, axis: str, color: str):
    fig, ax = plt.subplots(figsize=(8, 4))
    ax.plot(df["time_ms"], df[axis], color=color, linewidth=1.5)
    ax.set_xlabel("Time (ms)")
    ax.set_ylabel(f"{axis} (mg)")
    ax.set_title(f"{axis} vs Time")
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    return fig


def plot_all_axes_vs_time(df: pd.DataFrame):
    fig, ax = plt.subplots(figsize=(9, 5))
    ax.plot(df["time_ms"], df["acc_x"], color="tab:red", linewidth=1.5, label="acc_x")
    ax.plot(df["time_ms"], df["acc_y"], color="tab:green", linewidth=1.5, label="acc_y")
    ax.plot(df["time_ms"], df["acc_z"], color="tab:blue", linewidth=1.5, label="acc_z")
    ax.set_xlabel("Time (ms)")
    ax.set_ylabel("Acceleration (mg)")
    ax.set_title("All Axes vs Time")
    ax.legend()
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    return fig


def plot_3d_trajectory(df: pd.DataFrame):
    fig = plt.figure(figsize=(7, 6))
    ax = fig.add_subplot(111, projection="3d")

    ax.plot(df["acc_x"], df["acc_y"], df["acc_z"],
            color="tab:purple", linewidth=1.2)
    ax.scatter(df["acc_x"], df["acc_y"], df["acc_z"],
               c=df["time_ms"], cmap="viridis", s=15)

    ax.set_xlabel("acc_x (mg)")
    ax.set_ylabel("acc_y (mg)")
    ax.set_zlabel("acc_z (mg)")
    ax.set_title("3D Acceleration Trajectory")
    fig.tight_layout()
    return fig

def simple_moving_average_filter(df: pd.DataFrame) -> pd.DataFrame:
    """
    Applies a 3-point simple moving average (current + previous 2 samples)
    to acc_x, acc_y, acc_z. Returns a new DataFrame with smoothed values.
    The first 2 rows use whatever samples are available (min_periods=1),
    so no rows are dropped.
    """
    smoothed = df.copy()
    for axis in ("acc_x", "acc_y", "acc_z"):
        smoothed[axis] = df[axis].rolling(window=3, min_periods=1).mean()
    return smoothed


def main():
    if len(sys.argv) != 2:
        print("Usage: python plot_accel.py <path_to_csv>")
        sys.exit(1)

    filepath = sys.argv[1]
    df = load_data(filepath)

    fig_x = plot_axis_vs_time(simple_moving_average_filter(df), "acc_x", "tab:red")
    fig_y = plot_axis_vs_time(simple_moving_average_filter(df), "acc_y", "tab:green")
    fig_z = plot_axis_vs_time(simple_moving_average_filter(df), "acc_z", "tab:blue")
    fig_all = plot_all_axes_vs_time(simple_moving_average_filter(df))
    fig_3d = plot_3d_trajectory(simple_moving_average_filter(df))

    # Make sure the output folder exists, then save all figures into it
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    prefix = get_next_run_prefix(OUTPUT_DIR)

    fig_x.savefig(os.path.join(OUTPUT_DIR, f"{prefix}_x_vs_time.png"), dpi=150)
    fig_y.savefig(os.path.join(OUTPUT_DIR, f"{prefix}_y_vs_time.png"), dpi=150)
    fig_z.savefig(os.path.join(OUTPUT_DIR, f"{prefix}_z_vs_time.png"), dpi=150)
    fig_all.savefig(os.path.join(OUTPUT_DIR, f"{prefix}_all_vs_time.png"), dpi=150)
    fig_3d.savefig(os.path.join(OUTPUT_DIR, f"{prefix}_3d_trajectory.png"), dpi=150)

    print(f"Saved plots to '{OUTPUT_DIR}/' with prefix '{prefix}': "
          f"{prefix}_x_vs_time.png, {prefix}_y_vs_time.png, "
          f"{prefix}_z_vs_time.png, {prefix}_all_vs_time.png, "
          f"{prefix}_3d_trajectory.png")

    plt.show()


if __name__ == "__main__":
    main()