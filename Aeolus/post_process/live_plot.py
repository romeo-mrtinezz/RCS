import serial
import matplotlib.pyplot as plt
from collections import deque
import time


"""
Mode
1: acceleration
2: gyro
3: pitch, yaw
"""

MODE = 3

MODES = {
    1: {
        "signals": ["ax", "ay", "az"],
        "ylabel": "Acceleration (mg)",
        "ylim": (-2000, 2000),
    },

    2: {
        "signals": ["gx", "gy", "gz"],
        "ylabel": "Angular velocity (deg/s)",
        "ylim": (-500, 500),
    },

    3: {
        "signals": ["pitch", "yaw"],
        "ylabel": "Angle (deg)",
        "ylim": (-15, 15),
    },
}


# --------------------------------------------------
# Mode
# --------------------------------------------------

mode = MODES[MODE]
signals = mode["signals"]


# --------------------------------------------------
# Serial
# --------------------------------------------------

ser = serial.Serial(
    "COM3",
    115200,
    timeout=0.01
)

print("Connected to COM3")
print("Mode:", MODE)
print("Signals:", signals)


# --------------------------------------------------
# Data storage
# --------------------------------------------------

MAX_POINTS = 5000
WINDOW = 5  # seconds

samples = deque(maxlen=MAX_POINTS)


# --------------------------------------------------
# Plot setup
# --------------------------------------------------

plt.ion()

fig, ax = plt.subplots()

lines = {}

for signal in signals:

    lines[signal], = ax.plot(
        [],
        [],
        label=signal
    )


ax.set_xlabel("Time (s)")
ax.set_ylabel(mode["ylabel"])
ax.set_title("Live IMU Data")

ax.set_ylim(*mode["ylim"])

ax.legend()
ax.grid()


# --------------------------------------------------
# Main loop
# --------------------------------------------------

last_plot = time.perf_counter()

while True:

    try:

        # ------------------------------------------
        # Read all available serial data
        # ------------------------------------------

        while ser.in_waiting:

            line = ser.readline().decode(
                "utf-8",
                errors="ignore"
            ).strip()

            if not line:
                continue

            print(line)

            try:

                values = list(
                    map(float, line.split(","))
                )

                # First value is always timestamp
                # Remaining values are the signals

                expected_values = len(signals) + 1

                if len(values) != expected_values:

                    print("Invalid packet:", line)
                    continue

                # Store complete packet
                samples.append(values)

            except ValueError:

                print("Could not parse:", line)
                continue


        # ------------------------------------------
        # Update graph ~30 FPS
        # ------------------------------------------

        now = time.perf_counter()

        if now - last_plot >= 0.033:

            if len(samples) > 0:

                # Convert timestamp from ms -> s

                times = [
                    sample[0] / 1000.0
                    for sample in samples
                ]


                # Update each signal

                for i, signal in enumerate(signals):

                    values = [
                        sample[i + 1]
                        for sample in samples
                    ]

                    lines[signal].set_data(
                        times,
                        values
                    )


                # ----------------------------------
                # Rolling X axis
                # ----------------------------------

                if len(times) > 1:

                    current_time = times[-1]

                    ax.set_xlim(
                        max(0, current_time - WINDOW),
                        current_time
                    )


                # ----------------------------------
                # Fixed Y axis
                # ----------------------------------

                ax.set_ylim(*mode["ylim"])


                # ----------------------------------
                # Redraw
                # ----------------------------------

                fig.canvas.draw_idle()
                fig.canvas.flush_events()


            last_plot = now


        plt.pause(0.001)


    except KeyboardInterrupt:

        print("\nStopping...")
        break


ser.close()

print("Serial port closed.")

# calibrate gyro
# pitch yaw accel vs pitch gyro raw vs comp filt, GT 0, 45, 90