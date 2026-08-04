#!/usr/bin/env python3
"""
DISCLAIMER, THIS IS AI GENERATED
use for live telemetry data

plot_imu_logs.py

Reads accelerometer CSV logs from ../data (relative to this script's location
in post_process/), plots each axis vs time individually plus a combined plot,
and saves the PNGs into a new folder under data/ named after the log's
timestamp.

Expected CSV format (no header row):
    time_ms, acc_x, acc_y, acc_z

Expected filename format:
    COM5_2026_08_04.22.48.36.473

Usage:
    python plot_imu_logs.py                # process every .csv in ../data
    python plot_imu_logs.py somefile.csv   # process a single file (name or path)
"""

import sys
import re
import csv
from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt

SCRIPT_DIR = Path(__file__).resolve().parent
DATA_DIR = SCRIPT_DIR.parent / "data"

# Matches: COM5_2026_08_04.22.48.36.473  ->  2026_08_04.22.48.36.473
TIMESTAMP_RE = re.compile(r"(\d{4}_\d{2}_\d{2}\.\d{2}\.\d{2}\.\d{2}\.\d+)")


def get_output_folder_name(csv_path: Path) -> str:
    """Extract the timestamp from the filename to use as the output folder name.
    Falls back to the file stem if the pattern isn't found."""
    match = TIMESTAMP_RE.search(csv_path.stem)
    if match:
        return match.group(1)
    return csv_path.stem


def load_csv(csv_path: Path):
    """Load time_ms, acc_x, acc_y, acc_z columns from a headerless CSV."""
    time_ms, acc_x, acc_y, acc_z = [], [], [], []
    with open(csv_path, newline="") as f:
        reader = csv.reader(f)
        for row in reader:
            if not row or len(row) < 4:
                continue
            try:
                t, x, y, z = (float(v) for v in row[:4])
            except ValueError:
                # skip any stray header/malformed line
                continue
            time_ms.append(t)
            acc_x.append(x)
            acc_y.append(y)
            acc_z.append(z)

    return (
        np.array(time_ms),
        np.array(acc_x),
        np.array(acc_y),
        np.array(acc_z),
    )


def make_plots(csv_path: Path):
    time_ms, acc_x, acc_y, acc_z = load_csv(csv_path)

    if time_ms.size == 0:
        print(f"  [!] No valid data rows found in {csv_path.name}, skipping.")
        return

    # convert to seconds for nicer axis labels
    time_s = (time_ms - time_ms[0]) / 1000.0

    out_folder_name = get_output_folder_name(csv_path)
    out_dir = DATA_DIR / out_folder_name
    out_dir.mkdir(parents=True, exist_ok=True)

    axes_data = [
        ("acc_x", acc_x, "tab:red"),
        ("acc_y", acc_y, "tab:green"),
        ("acc_z", acc_z, "tab:blue"),
    ]

    # Individual plots
    for name, values, color in axes_data:
        plt.figure(figsize=(10, 5))
        plt.plot(time_s, values, color=color, linewidth=1)
        plt.title(f"{name} vs Time — {csv_path.name}")
        plt.xlabel("Time (s)")
        plt.ylabel(f"{name} (raw units)")
        plt.grid(True, alpha=0.3)
        plt.tight_layout()
        plt.savefig(out_dir / f"{name}.png", dpi=150)
        plt.close()

    # Combined plot
    plt.figure(figsize=(10, 5))
    for name, values, color in axes_data:
        plt.plot(time_s, values, color=color, linewidth=1, label=name)
    plt.title(f"All Axes vs Time — {csv_path.name}")
    plt.xlabel("Time (s)")
    plt.ylabel("Acceleration (raw units)")
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(out_dir / "combined.png", dpi=150)
    plt.close()

    print(f"  Saved plots to {out_dir}")


def resolve_csv_targets(arg: str | None):
    if arg is None:
        files = sorted(DATA_DIR.glob("*.csv"))
        if not files:
            print(f"No .csv files found in {DATA_DIR}")
        return files

    # allow either a bare filename (looked up in DATA_DIR) or a full/relative path
    candidate = Path(arg)
    if not candidate.is_absolute():
        if candidate.exists():
            return [candidate]
        candidate = DATA_DIR / arg
    if not candidate.exists():
        print(f"File not found: {arg}")
        return []
    return [candidate]


def main():
    arg = sys.argv[1] if len(sys.argv) > 1 else None
    targets = resolve_csv_targets(arg)

    for csv_path in targets:
        print(f"Processing {csv_path.name} ...")
        make_plots(csv_path)


if __name__ == "__main__":
    main()