#include "merge_sort.h"

#include <stdlib.h>

#include "shell_sort.h"

static void resetStats(SortStats *stats) {
	if (stats != NULL) {
		stats->comparisons = 0;
		stats->moves = 0;
		stats->auxiliary_bytes = 0;
	}
}

int mergeSortWithStats(int a[], int n, SortStats *stats) {
	resetStats(stats);
	if (n < 0 || (n > 0 && a == NULL)) {
		return 0;
	}
	if (n < 2) {
		return 1;
	}

	size_t length = (size_t)n;
	if (length > (size_t)-1 / sizeof(int)) {
		return 0;
	}

	int *buffer = malloc(length * sizeof(*buffer));
	if (buffer == NULL) {
		return 0;
	}
	if (stats != NULL) {
		stats->auxiliary_bytes = length * sizeof(*buffer);
	}

	for (size_t width = 1; width < length;) {
		size_t left = 0;
		while (left < length) {
			size_t middle = left + (width < length - left ? width : length - left);
			size_t right = middle + (width < length - middle ? width : length - middle);
			if (middle == right) {
				left = right;
				continue;
			}

			size_t first = left;
			size_t second = middle;
			size_t output = left;

			while (first < middle && second < right) {
				if (stats != NULL) {
					stats->comparisons++;
				}
				if (a[first] <= a[second]) {
					buffer[output++] = a[first++];
				} else {
					buffer[output++] = a[second++];
				}
				if (stats != NULL) {
					stats->moves++;
				}
			}
			while (first < middle) {
				buffer[output++] = a[first++];
				if (stats != NULL) {
					stats->moves++;
				}
			}
			while (second < right) {
				buffer[output++] = a[second++];
				if (stats != NULL) {
					stats->moves++;
				}
			}
			for (size_t index = left; index < right; index++) {
				a[index] = buffer[index];
				if (stats != NULL) {
					stats->moves++;
				}
			}
			left = right;
		}

		if (width > length / 2) {
			break;
		}
		width *= 2;
	}

	free(buffer);
	return 1;
}

void mergeSort(int a[], int n) {
	if (!mergeSortWithStats(a, n, NULL) && a != NULL && n > 1) {
		shellSort(a, n);
	}
}
