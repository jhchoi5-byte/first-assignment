#include "shell_sort.h"

static void resetStats(SortStats *stats) {
	if (stats != NULL) {
		stats->comparisons = 0;
		stats->moves = 0;
		stats->auxiliary_bytes = 0;
	}
}

void shellSortWithStats(int a[], int n, SortStats *stats) {
	resetStats(stats);
	if (a == NULL || n < 2) {
		return;
	}

	if (stats != NULL) {
		stats->auxiliary_bytes = sizeof(int);
	}

	for (int gap = n / 2; gap > 0; gap /= 2) {
		for (int index = gap; index < n; index++) {
			int value = a[index];
			if (stats != NULL) {
				stats->moves++;
			}

			int position = index;
			while (position >= gap) {
				if (stats != NULL) {
					stats->comparisons++;
				}
				if (a[position - gap] <= value) {
					break;
				}

				a[position] = a[position - gap];
				if (stats != NULL) {
					stats->moves++;
				}
				position -= gap;
			}

			a[position] = value;
			if (stats != NULL) {
				stats->moves++;
			}
		}
	}
}

void shellSort(int a[], int n) {
	shellSortWithStats(a, n, NULL);
}
