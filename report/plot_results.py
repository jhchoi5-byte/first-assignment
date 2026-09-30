#!/usr/bin/env python3
"""Create report-ready plots from the final benchmark summary CSV files."""

from __future__ import annotations

import csv
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

REPORT_DIR = Path(__file__).resolve().parent
RESULTS_DIR = REPORT_DIR / "results"
FIGURES_DIR = REPORT_DIR / "figures"

INPUT_SIZES = (500, 1000, 2000, 5000, 10000, 20000)
DISTRIBUTIONS = ("random", "sorted", "reverse", "duplicate-heavy")
ALGORITHMS = ("Shell Sort", "Merge Sort", "Library Sort")
COLORS = {
    "Shell Sort": "#0072B2",
    "Merge Sort": "#D55E00",
    "Library Sort": "#009E73",
}
MARKERS = {"Shell Sort": "o", "Merge Sort": "s", "Library Sort": "^"}
LABELS = {
    "Shell Sort": "Shell Sort",
    "Merge Sort": "Merge Sort",
    "Library Sort": "Library Sort (deterministic educational)",
}
X_POSITIONS = np.arange(len(INPUT_SIZES))
X_LABELS = [str(size) for size in INPUT_SIZES]


def read_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def load_runtime() -> dict[tuple[str, str], list[dict[str, float]]]:
    grouped: dict[tuple[str, str], list[dict[str, float]]] = defaultdict(list)
    for row in read_rows(RESULTS_DIR / "runtime_summary.csv"):
        algorithm = row["algorithm"]
        distribution = row["distribution"]
        size = int(row["input_size"])
        if algorithm not in ALGORITHMS or distribution not in DISTRIBUTIONS:
            raise ValueError(f"Unexpected runtime group: {algorithm}, {distribution}")
        if size not in INPUT_SIZES or int(row["trials"]) != 30:
            raise ValueError(f"Unexpected runtime size/trial count: {size}, {row['trials']}")

        values = {
            "input_size": size,
            "median": float(row["median_ns"]) / 1_000_000,
            "q1": float(row["q1_ns"]) / 1_000_000,
            "q3": float(row["q3_ns"]) / 1_000_000,
        }
        if values["q1"] <= 0 or not values["q1"] <= values["median"] <= values["q3"]:
            raise ValueError(f"Invalid runtime quartiles for {algorithm}, {size}, {distribution}")
        grouped[(algorithm, distribution)].append(values)

    for key, rows in grouped.items():
        rows.sort(key=lambda row: row["input_size"])
        if [int(row["input_size"]) for row in rows] != list(INPUT_SIZES):
            raise ValueError(f"Incomplete runtime size series: {key}")
    if len(grouped) != len(ALGORITHMS) * len(DISTRIBUTIONS):
        raise ValueError("Runtime summary does not contain every algorithm/distribution group")
    return grouped


def load_stats() -> dict[tuple[str, str], list[dict[str, float]]]:
    grouped: dict[tuple[str, str], list[dict[str, float]]] = defaultdict(list)
    for row in read_rows(RESULTS_DIR / "stats_summary.csv"):
        algorithm = row["algorithm"]
        distribution = row["distribution"]
        size = int(row["input_size"])
        if algorithm not in ALGORITHMS or distribution not in DISTRIBUTIONS:
            raise ValueError(f"Unexpected stats group: {algorithm}, {distribution}")
        if size not in INPUT_SIZES or int(row["trials"]) != 30:
            raise ValueError(f"Unexpected stats size/trial count: {size}, {row['trials']}")

        values = {
            "input_size": size,
            "comparisons": float(row["median_comparisons"]),
            "moves": float(row["median_moves"]),
            "auxiliary_bytes": float(row["median_auxiliary_bytes"]),
        }
        if min(values["comparisons"], values["moves"], values["auxiliary_bytes"]) <= 0:
            raise ValueError(f"Log-scale metric must be positive: {algorithm}, {size}, {distribution}")
        grouped[(algorithm, distribution)].append(values)

    for key, rows in grouped.items():
        rows.sort(key=lambda row: row["input_size"])
        if [int(row["input_size"]) for row in rows] != list(INPUT_SIZES):
            raise ValueError(f"Incomplete stats size series: {key}")
    if len(grouped) != len(ALGORITHMS) * len(DISTRIBUTIONS):
        raise ValueError("Stats summary does not contain every algorithm/distribution group")
    return grouped


def panel_figure(title: str, y_label: str):
    figure, axes = plt.subplots(2, 2, figsize=(12, 8), sharex=True, sharey=True)
    figure.suptitle(title, fontsize=16, fontweight="semibold", y=0.98)
    figure.supxlabel("Input size (elements)", fontsize=11)
    figure.supylabel(y_label, fontsize=11)

    for axis, distribution in zip(axes.flat, DISTRIBUTIONS):
        axis.set_title(distribution.replace("-", " ").title(), fontsize=12)
        axis.set_xticks(X_POSITIONS, X_LABELS)
        axis.set_yscale("log")
        axis.grid(True, which="major", color="#d7dde2", linewidth=0.7)
        axis.grid(True, which="minor", color="#edf0f2", linewidth=0.45, linestyle=":")
        axis.tick_params(axis="both", labelsize=9)

    return figure, axes


def add_panel_legend(figure, axes, footer: str) -> None:
    handles, labels = axes.flat[0].get_legend_handles_labels()
    figure.legend(handles, labels, loc="lower center", bbox_to_anchor=(0.5, 0.075),
                  ncols=3, frameon=False, fontsize=9)
    figure.text(0.5, 0.018, footer, ha="center", va="bottom", fontsize=8.5, color="#343a40")
    figure.tight_layout(rect=(0.055, 0.15, 0.99, 0.94))


def plot_runtime(runtime: dict[tuple[str, str], list[dict[str, float]]]) -> None:
    figure, axes = panel_figure(
        "Runtime by input distribution",
        "Runtime (ms, logarithmic scale)",
    )
    for axis, distribution in zip(axes.flat, DISTRIBUTIONS):
        for algorithm in ALGORITHMS:
            rows = runtime[(algorithm, distribution)]
            medians = np.array([row["median"] for row in rows])
            lower = medians - np.array([row["q1"] for row in rows])
            upper = np.array([row["q3"] for row in rows]) - medians
            axis.errorbar(
                X_POSITIONS,
                medians,
                yerr=np.vstack((lower, upper)),
                color=COLORS[algorithm],
                marker=MARKERS[algorithm],
                markersize=5,
                linewidth=1.8,
                capsize=3,
                elinewidth=1.0,
                label=LABELS[algorithm],
            )
    add_panel_legend(
        figure,
        axes,
        "Library Sort is a deterministic educational implementation; these results do not represent randomized Library Sort.",
    )
    save_figure(figure, "runtime")


def plot_stat_metric(
    stats: dict[tuple[str, str], list[dict[str, float]]],
    metric: str,
    title: str,
    y_label: str,
    filename: str,
    footer: str,
) -> None:
    figure, axes = panel_figure(title, y_label)
    for axis, distribution in zip(axes.flat, DISTRIBUTIONS):
        for algorithm in ALGORITHMS:
            rows = stats[(algorithm, distribution)]
            axis.plot(
                X_POSITIONS,
                [row[metric] for row in rows],
                color=COLORS[algorithm],
                marker=MARKERS[algorithm],
                markersize=5,
                linewidth=1.8,
                label=LABELS[algorithm],
            )
    add_panel_legend(figure, axes, footer)
    save_figure(figure, filename)


def plot_auxiliary_memory(stats: dict[tuple[str, str], list[dict[str, float]]]) -> None:
    figure, axis = plt.subplots(figsize=(9, 5.5))
    figure.suptitle("Auxiliary memory by input size", fontsize=16, fontweight="semibold", y=0.98)
    axis.set_xlabel("Input size (elements)", fontsize=11)
    axis.set_ylabel("Auxiliary allocation (bytes, logarithmic scale)", fontsize=11)
    axis.set_xticks(X_POSITIONS, X_LABELS)
    axis.set_yscale("log")
    axis.grid(True, which="major", color="#d7dde2", linewidth=0.7)
    axis.grid(True, which="minor", color="#edf0f2", linewidth=0.45, linestyle=":")

    all_values = []
    for algorithm in ALGORITHMS:
        values = []
        for size in INPUT_SIZES:
            per_distribution = [
                next(row["auxiliary_bytes"] for row in stats[(algorithm, distribution)]
                     if row["input_size"] == size)
                for distribution in DISTRIBUTIONS
            ]
            if len(set(per_distribution)) != 1:
                raise ValueError(f"Auxiliary memory differs by distribution: {algorithm}, {size}")
            values.append(per_distribution[0])
            all_values.append(per_distribution[0])
        axis.plot(
            X_POSITIONS,
            values,
            color=COLORS[algorithm],
            marker=MARKERS[algorithm],
            markersize=6,
            linewidth=2,
            label=LABELS[algorithm],
        )

    axis.set_ylim(min(all_values) * 0.7, max(all_values) * 1.5)
    axis.legend(frameon=False, ncols=1, loc="best", fontsize=9)
    figure.text(
        0.5,
        0.015,
        "Recorded auxiliary allocation only; excludes allocator overhead and process RSS. "
        "Library Sort is a deterministic educational implementation, not a randomized variant.",
        ha="center",
        va="bottom",
        fontsize=8.5,
        color="#343a40",
    )
    figure.tight_layout(rect=(0.04, 0.075, 0.99, 0.93))
    save_figure(figure, "auxiliary_memory")


def save_figure(figure, name: str) -> None:
    FIGURES_DIR.mkdir(parents=True, exist_ok=True)
    figure.savefig(FIGURES_DIR / f"{name}.png", dpi=300, bbox_inches="tight", facecolor="white")
    figure.savefig(FIGURES_DIR / f"{name}.pdf", bbox_inches="tight", facecolor="white")
    plt.close(figure)
    print(f"Created {FIGURES_DIR / (name + '.png')}")
    print(f"Created {FIGURES_DIR / (name + '.pdf')}")


def main() -> None:
    plt.rcParams.update({
        "font.family": "DejaVu Sans",
        "axes.spines.top": False,
        "axes.spines.right": False,
        "pdf.fonttype": 42,
        "ps.fonttype": 42,
        "savefig.dpi": 300,
    })
    runtime = load_runtime()
    stats = load_stats()

    plot_runtime(runtime)
    plot_stat_metric(
        stats,
        "comparisons",
        "Recorded key comparisons by input distribution",
        "Recorded key comparisons (logarithmic scale)",
        "comparisons",
        "Library Sort comparisons exclude occupied-slot checks during gap search; counts are not equivalent machine-operation totals.",
    )
    plot_stat_metric(
        stats,
        "moves",
        "Recorded element moves by input distribution",
        "Recorded moves (logarithmic scale)",
        "moves",
        "Move counts follow each implementation's accounting rules; Library Sort is a deterministic educational variant.",
    )
    plot_auxiliary_memory(stats)


if __name__ == "__main__":
    main()
