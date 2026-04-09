#!/usr/bin/env python3
"""Generate plots for spring simulations from CSV logs.

Reads:
- ./out/spring.csv
- ./out/constant_force.csv
- ./out/sinusoidal_force.csv

Creates:
- ./out/spring_plot.png
- ./out/spring_state_space_plot.png
- ./out/constant_force_plot.png
- ./out/constant_force_state_space_plot.png
- ./out/sinusoidal_force_plot.png
- ./out/sinusoidal_force_state_space_plot.png
"""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt


def load_data(csv_path: Path) -> tuple[list[float], list[float], list[float]]:
    """Load t, x0, x1 columns from a CSV file."""
    t_values: list[float] = []
    x0_values: list[float] = []
    x1_values: list[float] = []

    with csv_path.open("r", newline="", encoding="utf-8") as csv_file:
        reader = csv.DictReader(csv_file)
        required_columns = {"t", "x0", "x1"}
        if not required_columns.issubset(reader.fieldnames or []):
            raise ValueError(f"Missing required columns in {csv_path.name}: {required_columns}")

        for row in reader:
            t_values.append(float(row["t"]))
            x0_values.append(float(row["x0"]))
            x1_values.append(float(row["x1"]))

    return t_values, x0_values, x1_values


def make_plot(simulation_name: str, csv_path: Path, output_path: Path, show_plot: bool) -> None:
    """Create and save time-series and state-space plots for one simulation."""
    t_values, x0_values, x1_values = load_data(csv_path)

    plt.figure(figsize=(10, 6))
    plt.plot(t_values, x0_values, label="x0 (position)", linewidth=2.0)
    plt.plot(t_values, x1_values, label="x1 (velocity)", linewidth=2.0)

    plt.title(f"{simulation_name}: Time vs Position and Velocity")
    plt.xlabel("t [s]")
    plt.ylabel("state value")
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()

    plt.savefig(output_path, dpi=160)
    if show_plot:
        plt.show()
    plt.close()

    state_space_output_path = output_path.with_name(
        output_path.stem.replace("_plot", "_state_space_plot") + output_path.suffix
    )

    plt.figure(figsize=(8, 8))
    plt.plot(x0_values, x1_values, linewidth=2.0)

    plt.title(f"{simulation_name}: State Space")
    plt.xlabel("x0 (position)")
    plt.ylabel("x1 (velocity)")
    plt.grid(True, alpha=0.3)
    plt.tight_layout()

    plt.savefig(state_space_output_path, dpi=160)
    if show_plot:
        plt.show()
    plt.close()


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate plots for spring, constant_force and sinusoidal_force simulations."
    )
    parser.add_argument(
        "--show",
        action="store_true",
        help="Display plots interactively in addition to saving PNG files.",
    )
    args = parser.parse_args()

    base_dir = Path(__file__).resolve().parent
    out_dir = base_dir / "out"

    simulation_files = {
        "spring": out_dir / "spring.csv",
        "constant_force": out_dir / "constant_force.csv",
        "sinusoidal_force": out_dir / "sinusoidal_force.csv",
    }

    for simulation_name, csv_path in simulation_files.items():
        if not csv_path.exists():
            raise FileNotFoundError(f"CSV file not found: {csv_path}")

        output_path = out_dir / f"{simulation_name}_plot.png"
        make_plot(simulation_name, csv_path, output_path, args.show)
        print(f"Saved: {output_path}")


if __name__ == "__main__":
    main()
