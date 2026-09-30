#include "library_sort.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    int value;
    bool occupied;
} LibrarySlot;

static void resetStats(SortStats *stats) {
    if (stats != NULL) {
        stats->comparisons = 0;
        stats->moves = 0;
        stats->auxiliary_bytes = 0;
    }
}

static size_t findInsertionBoundary(const LibrarySlot slots[], size_t capacity,
                                    int value, SortStats *stats) {
    size_t boundary = 0;

    for (size_t index = 0; index < capacity; index++) {
        if (!slots[index].occupied) {
            continue;
        }
        if (stats != NULL) {
            stats->comparisons++;
        }
        if (slots[index].value >= value) {
            return index;
        }
        boundary = index + 1;
    }

    return boundary;
}

static bool findNearestEmpty(const LibrarySlot slots[], size_t capacity,
                             size_t boundary, size_t *emptyIndex) {
    size_t left = boundary;
    size_t right = boundary;

    while (left > 0 || right < capacity) {
        if (right < capacity) {
            if (!slots[right].occupied) {
                *emptyIndex = right;
                return true;
            }
            right++;
        }
        if (left > 0) {
            left--;
            if (!slots[left].occupied) {
                *emptyIndex = left;
                return true;
            }
        }
    }

    return false;
}

static size_t openInsertionGap(LibrarySlot slots[], size_t boundary,
                               size_t emptyIndex, SortStats *stats) {
    if (emptyIndex == boundary) {
        return boundary;
    }

    if (emptyIndex > boundary) {
        for (size_t index = emptyIndex; index > boundary; index--) {
            slots[index] = slots[index - 1];
            if (stats != NULL) {
                stats->moves++;
            }
        }
        slots[boundary].occupied = false;
        return boundary;
    }

    for (size_t index = emptyIndex; index + 1 < boundary; index++) {
        slots[index] = slots[index + 1];
        if (stats != NULL) {
            stats->moves++;
        }
    }
    slots[boundary - 1].occupied = false;
    return boundary - 1;
}

static void rebalanceSlots(LibrarySlot slots[], size_t capacity, int scratch[],
                           size_t count, SortStats *stats) {
    size_t output = 0;
    for (size_t index = 0; index < capacity; index++) {
        if (slots[index].occupied) {
            scratch[output++] = slots[index].value;
            if (stats != NULL) {
                stats->moves++;
            }
            slots[index].occupied = false;
        }
    }

    size_t emptyCount = capacity - count;
    size_t gapSize = emptyCount / (count + 1);
    size_t extraGaps = emptyCount % (count + 1);
    size_t position = 0;

    for (size_t index = 0; index < count; index++) {
        position += gapSize + (index < extraGaps ? 1 : 0);
        slots[position].value = scratch[index];
        slots[position].occupied = true;
        if (stats != NULL) {
            stats->moves++;
        }
        position++;
    }
}

static bool insertValue(LibrarySlot slots[], size_t capacity, int value,
                        SortStats *stats) {
    size_t boundary = findInsertionBoundary(slots, capacity, value, stats);
    size_t emptyIndex;
    if (!findNearestEmpty(slots, capacity, boundary, &emptyIndex)) {
        return false;
    }

    size_t insertionIndex = openInsertionGap(slots, boundary, emptyIndex, stats);
    slots[insertionIndex].value = value;
    slots[insertionIndex].occupied = true;
    if (stats != NULL) {
        stats->moves++;
    }
    return true;
}

int librarySortWithStats(int a[], int n, SortStats *stats) {
    resetStats(stats);
    if (n < 0 || (n > 0 && a == NULL)) {
        return 0;
    }
    if (n < 2) {
        return 1;
    }

    size_t length = (size_t)n;
    if (length > SIZE_MAX / 2) {
        return 0;
    }
    size_t capacity = length * 2;
    if (capacity > SIZE_MAX / sizeof(LibrarySlot) ||
        length > SIZE_MAX / sizeof(int)) {
        return 0;
    }

    size_t slotsBytes = capacity * sizeof(LibrarySlot);
    size_t scratchBytes = length * sizeof(int);
    if (slotsBytes > SIZE_MAX - scratchBytes) {
        return 0;
    }

    LibrarySlot *slots = calloc(capacity, sizeof(*slots));
    if (slots == NULL) {
        return 0;
    }
    int *scratch = malloc(scratchBytes);
    if (scratch == NULL) {
        free(slots);
        return 0;
    }
    if (stats != NULL) {
        stats->auxiliary_bytes = slotsBytes + scratchBytes;
    }

    size_t inserted = 0;
    size_t batchSize = 1;
    while (inserted < length) {
        size_t remaining = length - inserted;
        size_t batchCount = batchSize < remaining ? batchSize : remaining;

        for (size_t index = 0; index < batchCount; index++) {
            if (!insertValue(slots, capacity, a[inserted + index], stats)) {
                rebalanceSlots(slots, capacity, scratch, inserted + index, stats);
                if (!insertValue(slots, capacity, a[inserted + index], stats)) {
                    free(scratch);
                    free(slots);
                    resetStats(stats);
                    return 0;
                }
            }
        }
        inserted += batchCount;

        if (inserted < length) {
            rebalanceSlots(slots, capacity, scratch, inserted, stats);
        }
        if (batchSize <= length / 2) {
            batchSize *= 2;
        } else {
            batchSize = length;
        }
    }

    size_t output = 0;
    for (size_t index = 0; index < capacity; index++) {
        if (slots[index].occupied) {
            a[output++] = slots[index].value;
            if (stats != NULL) {
                stats->moves++;
            }
        }
    }

    free(scratch);
    free(slots);
    return output == length;
}

void librarySort(int a[], int n) {
    (void)librarySortWithStats(a, n, NULL);
}
