# Sorting Project

This directory contains the project-specific algorithms, integrated tests, benchmark outputs, and figures. The repository-root Makefile remains the build and test entry point.

## Contents

- `shell_sort.c`, `merge_sort.c`, `library_sort.c`: sorting implementations and matching headers
- `sort_stats.h`: operation counters shared by the algorithms
- `tests/test_sort.c`: C tests for Bubble, Shell, Merge, and Library Sort
- `benchmark.c`: pilot and final runtime/statistics benchmark
- `results/`: raw pilot/final CSV data and grouped summary CSV data
- `plot_results.py`: figure generation from the summary CSV files
- `figures/`: PNG and PDF figures
- `requirements-plot.txt`: plotting dependency

## Commands

Run tests from the repository root:

```sh
make test
```

Run a benchmark only when a new measurement is explicitly intended:

```sh
make benchmark-pilot
make benchmark-final
```

Regenerate the figures from the existing summary CSV files:

```sh
python3 report/plot_results.py
```
