import re
import numpy as np
import matplotlib.pyplot as plt


# ============================================================
# SETTINGS
# ============================================================

THRUST_THRESHOLD = 0.8  # Newtons


# ============================================================
# GET FILE NAME
# ============================================================

filename = input("Enter filename (including .txt): ")


# ============================================================
# READ DATA
# ============================================================

time_ms = []
low_side_pressure = []
weight_raw_kg = []

with open(filename, "r") as file:
    for line in file:

        # Extract all numbers from the line
        values = re.findall(r"[-+]?\d*\.?\d+", line)

        # We need at least:
        # timestep, high pressure, low pressure
        if len(values) < 3:
            continue

        try:
            time_ms.append(float(values[0]))
            low_side_pressure.append(float(values[2]))

            # Weight may occasionally not be transmitted.
            # If it exists, store it.
            # Otherwise store NaN.
            if len(values) >= 4:
                weight_raw_kg.append(float(values[3]))
            else:
                weight_raw_kg.append(np.nan)

        except ValueError:
            continue


# ============================================================
# CONVERT TO NUMPY ARRAYS
# ============================================================

time_ms = np.array(time_ms)
low_side_pressure = np.array(low_side_pressure)
weight_raw_kg = np.array(weight_raw_kg)


# ============================================================
# CONVERT TIME
# ============================================================

time = time_ms / 1000.0


# ============================================================
# CHECK AVAILABLE TIME RANGE
# ============================================================

if len(time) == 0:
    print("No valid data found in file.")
    exit()

available_start = time[0]
available_end = time[-1]

print("\n==============================")
print("AVAILABLE TIME RANGE")
print("==============================")
print(f"Start time: {available_start:.3f} s")
print(f"End time:   {available_end:.3f} s")
print(f"Timestep:   {np.median(np.diff(time)):.3f} s")


# ============================================================
# GET GRAPH TIME RANGE
# ============================================================

while True:

    start_time = float(
        input("\nEnter graph start time (s): ")
    )

    end_time = float(
        input("Enter graph end time (s): ")
    )

    if start_time < available_start:
        print(
            f"Start time is before available data "
            f"({available_start:.3f} s)."
        )
        continue

    if end_time > available_end:
        print(
            f"End time is after available data "
            f"({available_end:.3f} s)."
        )
        continue

    if start_time >= end_time:
        print("Start time must be less than end time.")
        continue

    break


# ============================================================
# CHECK WHETHER WEIGHT DATA EXISTS
# ============================================================

weight_data_available = np.any(
    ~np.isnan(weight_raw_kg)
)

if weight_data_available:

    print("\nValid weight data detected.")

else:

    print("\nWARNING: No valid weight data detected.")
    print("Weight/force/impulse graphs will be skipped.")


# ============================================================
# CALIBRATE WEIGHT
# ============================================================

if weight_data_available:

    # Raw load-cell value is in kg
    weight_raw_g = weight_raw_kg * 1000.0

    # Calibration:
    # actual weight (g) =
    # (measured weight (g) - 0.727206) / 3.503136

    calibrated_weight_g = (
        weight_raw_g - 0.727206
    ) / 3.503136

    calibrated_weight_kg = (
        calibrated_weight_g / 1000.0
    )

else:

    # Create an array of NaNs so the variables still exist
    calibrated_weight_kg = np.full(
        len(time),
        np.nan
    )


# ============================================================
# CALCULATE FORCE
# ============================================================

if weight_data_available:

    force = calibrated_weight_kg * 9.81

else:

    force = np.full(
        len(time),
        np.nan
    )


# ============================================================
# DETECT THRUST BURSTS
# ============================================================

burst_impulses = []
burst_durations = []
burst_start_times = []
burst_end_times = []

if weight_data_available:

    in_burst = False
    burst_impulse = 0.0
    burst_start_time = None

    for i in range(1, len(time)):

        dt = time[i] - time[i - 1]

        # Check whether both samples contain valid
        # load-cell data.
        current_valid = np.isfinite(force[i])
        previous_valid = np.isfinite(force[i - 1])

        # --------------------------------------------------------
        # MISSING WEIGHT DATA
        # --------------------------------------------------------

        if not current_valid or not previous_valid:

            # A missing transmission breaks a thrust burst.
            if in_burst:

                burst_end_time = time[i - 1]

                burst_impulses.append(
                    burst_impulse
                )

                burst_durations.append(
                    burst_end_time - burst_start_time
                )

                burst_start_times.append(
                    burst_start_time
                )

                burst_end_times.append(
                    burst_end_time
                )

                in_burst = False
                burst_impulse = 0.0
                burst_start_time = None

            continue


        # --------------------------------------------------------
        # START OF BURST
        # --------------------------------------------------------

        if not in_burst:

            if force[i] > THRUST_THRESHOLD:

                in_burst = True

                burst_start_time = time[i]

                burst_impulse = 0.0

                # First trapezoid
                burst_impulse += (
                    (force[i] + force[i - 1]) / 2.0
                ) * dt


        # --------------------------------------------------------
        # INSIDE BURST
        # --------------------------------------------------------

        else:

            # Integrate force using trapezoidal rule
            burst_impulse += (
                (force[i] + force[i - 1]) / 2.0
            ) * dt


            # ----------------------------------------------------
            # END OF BURST
            # ----------------------------------------------------

            if force[i] <= THRUST_THRESHOLD:

                burst_end_time = time[i]

                burst_impulses.append(
                    burst_impulse
                )

                burst_durations.append(
                    burst_end_time - burst_start_time
                )

                burst_start_times.append(
                    burst_start_time
                )

                burst_end_times.append(
                    burst_end_time
                )

                in_burst = False

                burst_impulse = 0.0
                burst_start_time = None


    # ============================================================
    # HANDLE BURST EXTENDING TO END OF DATA
    # ============================================================

    if in_burst:

        burst_end_time = time[-1]

        burst_impulses.append(
            burst_impulse
        )

        burst_durations.append(
            burst_end_time - burst_start_time
        )

        burst_start_times.append(
            burst_start_time
        )

        burst_end_times.append(
            burst_end_time
        )


# ============================================================
# PRINT BURST INFORMATION
# ============================================================

if weight_data_available:

    print("\n==============================")
    print("THRUST BURST RESULTS")
    print("==============================")

    if len(burst_impulses) == 0:

        print("No thrust bursts detected.")

    else:

        for i in range(len(burst_impulses)):

            print(
                f"Burst {i + 1}: "
                f"Impulse = {burst_impulses[i]:.3f} Ns, "
                f"Duration = {burst_durations[i]:.3f} s, "
                f"Start = {burst_start_times[i]:.3f} s, "
                f"End = {burst_end_times[i]:.3f} s"
            )

        print(
            f"\nTotal impulse = "
            f"{sum(burst_impulses):.3f} Ns"
        )


# ============================================================
# CREATE TIME MASK
# ============================================================

time_mask = (
    (time >= start_time) &
    (time <= end_time)
)

plot_time = time[time_mask]
plot_pressure = low_side_pressure[time_mask]


if weight_data_available:

    plot_weight = calibrated_weight_kg[time_mask]
    plot_force = force[time_mask]


# ============================================================
# GRAPH 1
# LOW-SIDE PRESSURE VS TIME
# ============================================================

plt.figure()

plt.plot(
    plot_time,
    plot_pressure,
    color="blue"
)

plt.xlabel("Time (s)")
plt.ylabel("Low-side pressure (bar)")
plt.title("Low-side Pressure vs Time")

plt.xlim(start_time, end_time)

plt.grid(True)

plt.tight_layout()


# ============================================================
# WEIGHT-DEPENDENT GRAPHS
# ============================================================

if weight_data_available:

    # ========================================================
    # GRAPH 2
    # FORCE VS TIME
    # ========================================================

    plt.figure()

    plt.plot(
        plot_time,
        plot_force,
        color="red"
    )

    plt.xlabel("Time (s)")
    plt.ylabel("Force (N)")
    plt.title("Force vs Time")

    plt.xlim(start_time, end_time)

    plt.grid(True)

    plt.tight_layout()


    # ========================================================
    # GRAPH 3
    # FORCE + LOW-SIDE PRESSURE
    # ========================================================

    fig, ax1 = plt.subplots()

    # BLUE = force
    ax1.plot(
        plot_time,
        plot_force,
        color="blue",
        label="Force"
    )

    ax1.set_xlabel("Time (s)")

    ax1.set_ylabel(
        "Force (N)",
        color="blue"
    )

    ax1.tick_params(
        axis="y",
        labelcolor="blue"
    )

    ax1.grid(True)


    # ORANGE = low-side pressure
    ax2 = ax1.twinx()

    ax2.plot(
        plot_time,
        plot_pressure,
        color="orange",
        label="Low-side pressure"
    )

    ax2.set_ylabel(
        "Low-side pressure (bar)",
        color="orange"
    )

    ax2.tick_params(
        axis="y",
        labelcolor="orange"
    )

    plt.title(
        "Force and Low-side Pressure vs Time"
    )

    ax1.set_xlim(
        start_time,
        end_time
    )

    fig.tight_layout()


    # ========================================================
    # GRAPH 4
    # IMPULSE PER THRUST BURST
    # ========================================================

    plt.figure()

    # Only show bursts that overlap
    # the selected time range
    selected_impulses = []
    selected_burst_numbers = []

    for i in range(len(burst_impulses)):

        if (
            burst_end_times[i] >= start_time
            and
            burst_start_times[i] <= end_time
        ):

            selected_impulses.append(
                burst_impulses[i]
            )

            selected_burst_numbers.append(
                len(selected_impulses)
            )


    if len(selected_impulses) > 0:

        bars = plt.bar(
            selected_burst_numbers,
            selected_impulses
        )

        # Add value above each bar
        for bar, impulse in zip(
            bars,
            selected_impulses
        ):

            plt.text(
                bar.get_x() + bar.get_width() / 2,
                bar.get_height(),
                f"{impulse:.2f}",
                ha="center",
                va="bottom"
            )

        plt.xlabel("Thrust burst")
        plt.ylabel("Impulse (Ns)")
        plt.title("Impulse per Thrust Burst")

        plt.xticks(
            selected_burst_numbers
        )

    else:

        plt.text(
            0.5,
            0.5,
            "No thrust bursts detected in selected time range",
            ha="center",
            va="center",
            transform=plt.gca().transAxes
        )

        plt.title(
            "Impulse per Thrust Burst"
        )

    plt.grid(axis="y")

    plt.tight_layout()


# ============================================================
# SHOW ALL GRAPHS
# ============================================================

plt.show()
