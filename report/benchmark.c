#define _POSIX_C_SOURCE 200809L

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "library_sort.h"
#include "merge_sort.h"
#include "shell_sort.h"

#define MASTER_SEED UINT64_C(20260930)
#define ALGORITHM_COUNT 3
#define PERMUTATION_COUNT 6

static const int inputSizes[] = {500, 1000, 2000, 5000, 10000, 20000};
static const char *distributionNames[] = {
    "random", "sorted", "reverse", "duplicate-heavy"
};
static const char *algorithmNames[] = {
    "Shell Sort", "Merge Sort", "Library Sort"
};
static const char *permutationNames[] = {
    "SML", "SLM", "MSL", "MLS", "LSM", "LMS"
};
static const unsigned char permutations[PERMUTATION_COUNT][ALGORITHM_COUNT] = {
    {0, 1, 2},
    {0, 2, 1},
    {1, 0, 2},
    {1, 2, 0},
    {2, 0, 1},
    {2, 1, 0}
};

typedef void (*SortFunction)(int[], int);

static SortFunction runtimeSorts[ALGORITHM_COUNT] = {
    shellSort, mergeSort, librarySort
};

static uint64_t mixSeed(uint64_t value) {
    value += UINT64_C(0x9E3779B97F4A7C15);
    value = (value ^ (value >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94D049BB133111EB);
    return value ^ (value >> 31);
}

static uint64_t seedForCase(size_t sizeIndex, size_t distributionIndex, int trial) {
    uint64_t seed = mixSeed(MASTER_SEED ^ (uint64_t)sizeIndex);
    seed = mixSeed(seed ^ (uint64_t)distributionIndex);
    return mixSeed(seed ^ (uint64_t)trial);
}

static uint64_t nextRandom(uint64_t *state) {
    *state += UINT64_C(0x9E3779B97F4A7C15);
    return mixSeed(*state);
}

static void generateInput(int values[], int count, int distribution, uint64_t seed) {
    uint64_t randomState = seed;

    if (distribution == 0) {
        for (int index = 0; index < count; index++) {
            values[index] = (int)(nextRandom(&randomState) % UINT64_C(2000001)) - 1000000;
        }
        return;
    }

    if (distribution == 1 || distribution == 2) {
        for (int index = 0; index < count; index++) {
            int offset = (int)(nextRandom(&randomState) % UINT64_C(3));
            values[index] = index * 3 + offset - count * 3 / 2;
        }
        if (distribution == 2) {
            for (int left = 0, right = count - 1; left < right; left++, right--) {
                int temporary = values[left];
                values[left] = values[right];
                values[right] = temporary;
            }
        }
        return;
    }

    for (int index = 0; index < count; index++) {
        values[index] = (int)(nextRandom(&randomState) % UINT64_C(21)) - 10;
    }
}

static int compareInts(const void *left, const void *right) {
    int leftValue = *(const int *)left;
    int rightValue = *(const int *)right;
    return (leftValue > rightValue) - (leftValue < rightValue);
}

static bool matchesOracle(const int values[], const int oracle[], int count) {
    for (int index = 0; index < count; index++) {
        if (values[index] != oracle[index]) {
            return false;
        }
    }
    return true;
}

static bool elapsedNanoseconds(const struct timespec *start,
                              const struct timespec *end,
                              uint64_t *elapsed) {
    int64_t seconds = (int64_t)end->tv_sec - (int64_t)start->tv_sec;
    int64_t nanoseconds = (int64_t)end->tv_nsec - (int64_t)start->tv_nsec;

    if (nanoseconds < 0) {
        seconds--;
        nanoseconds += INT64_C(1000000000);
    }
    if (seconds < 0) {
        return false;
    }

    *elapsed = (uint64_t)seconds * UINT64_C(1000000000) + (uint64_t)nanoseconds;
    return true;
}

static bool prepareCase(int source[], int oracle[], int count, size_t sizeIndex,
                        size_t distributionIndex, int trial, uint64_t *caseSeed) {
    *caseSeed = seedForCase(sizeIndex, distributionIndex, trial);
    generateInput(source, count, (int)distributionIndex, *caseSeed);
    if (count > 0) {
        memcpy(oracle, source, (size_t)count * sizeof(*oracle));
        qsort(oracle, (size_t)count, sizeof(*oracle), compareInts);
    }
    return true;
}

static bool runRuntime(FILE *csv, int trials) {
    const size_t sizeCount = sizeof(inputSizes) / sizeof(inputSizes[0]);
    const size_t distributionCount = sizeof(distributionNames) / sizeof(distributionNames[0]);
    int maximumSize = inputSizes[sizeCount - 1];
    int *source = malloc((size_t)maximumSize * sizeof(*source));
    int *oracle = malloc((size_t)maximumSize * sizeof(*oracle));
    int *working = malloc((size_t)maximumSize * sizeof(*working));
    bool allCorrect = true;

    if (source == NULL || oracle == NULL || working == NULL) {
        free(working);
        free(oracle);
        free(source);
        return false;
    }

    fprintf(csv, "algorithm,input_size,distribution,trial,master_seed,seed,run_order,run_position,elapsed_ns,correct\n");
    for (size_t sizeIndex = 0; sizeIndex < sizeCount; sizeIndex++) {
        int count = inputSizes[sizeIndex];
        for (size_t distribution = 0; distribution < distributionCount; distribution++) {
            for (int trial = 0; trial < trials; trial++) {
                uint64_t caseSeed;
                prepareCase(source, oracle, count, sizeIndex, distribution, trial, &caseSeed);
                size_t permutationIndex = (size_t)trial % PERMUTATION_COUNT;
                const unsigned char *order = permutations[permutationIndex];

                for (int position = 0; position < ALGORITHM_COUNT; position++) {
                    int algorithm = order[position];
                    memcpy(working, source, (size_t)count * sizeof(*working));

                    struct timespec start;
                    struct timespec end;
                    uint64_t elapsed;
                    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
                        allCorrect = false;
                        continue;
                    }
                    runtimeSorts[algorithm](working, count);
                    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0 ||
                        !elapsedNanoseconds(&start, &end, &elapsed)) {
                        allCorrect = false;
                        continue;
                    }

                    bool correct = matchesOracle(working, oracle, count);
                    fprintf(csv, "%s,%d,%s,%d,%" PRIu64 ",%" PRIu64 ",%s,%d,%" PRIu64 ",%d\n",
                            algorithmNames[algorithm], count, distributionNames[distribution],
                            trial, MASTER_SEED, caseSeed, permutationNames[permutationIndex],
                            position, elapsed, correct ? 1 : 0);
                    if (!correct) {
                        allCorrect = false;
                    }
                }
            }
        }
    }

    free(working);
    free(oracle);
    free(source);
    return allCorrect;
}

static bool runStatsForAlgorithm(int algorithm, int values[], int count, SortStats *stats) {
    if (algorithm == 0) {
        shellSortWithStats(values, count, stats);
        return true;
    }
    if (algorithm == 1) {
        return mergeSortWithStats(values, count, stats) != 0;
    }
    return librarySortWithStats(values, count, stats) != 0;
}

static bool runStats(FILE *csv, int trials) {
    const size_t sizeCount = sizeof(inputSizes) / sizeof(inputSizes[0]);
    const size_t distributionCount = sizeof(distributionNames) / sizeof(distributionNames[0]);
    int maximumSize = inputSizes[sizeCount - 1];
    int *source = malloc((size_t)maximumSize * sizeof(*source));
    int *oracle = malloc((size_t)maximumSize * sizeof(*oracle));
    int *working = malloc((size_t)maximumSize * sizeof(*working));
    bool allCorrect = true;

    if (source == NULL || oracle == NULL || working == NULL) {
        free(working);
        free(oracle);
        free(source);
        return false;
    }

    fprintf(csv, "algorithm,input_size,distribution,trial,master_seed,seed,comparisons,moves,auxiliary_bytes,correct,status\n");
    for (size_t sizeIndex = 0; sizeIndex < sizeCount; sizeIndex++) {
        int count = inputSizes[sizeIndex];
        for (size_t distribution = 0; distribution < distributionCount; distribution++) {
            for (int trial = 0; trial < trials; trial++) {
                uint64_t caseSeed;
                prepareCase(source, oracle, count, sizeIndex, distribution, trial, &caseSeed);

                for (int algorithm = 0; algorithm < ALGORITHM_COUNT; algorithm++) {
                    memcpy(working, source, (size_t)count * sizeof(*working));
                    SortStats stats;
                    bool succeeded = runStatsForAlgorithm(algorithm, working, count, &stats);
                    bool correct = succeeded && matchesOracle(working, oracle, count);
                    fprintf(csv, "%s,%d,%s,%d,%" PRIu64 ",%" PRIu64 ",%zu,%zu,%zu,%d,%s\n",
                            algorithmNames[algorithm], count, distributionNames[distribution],
                            trial, MASTER_SEED, caseSeed, stats.comparisons, stats.moves,
                            stats.auxiliary_bytes, correct ? 1 : 0,
                            succeeded ? "success" : "failure");
                    if (!correct) {
                        allCorrect = false;
                    }
                }
            }
        }
    }

    free(working);
    free(oracle);
    free(source);
    return allCorrect;
}

int main(int argc, char *argv[]) {
    bool pilotMode;
    int trials;
    const char *runtimePath;
    const char *statsPath;

    if (argc != 2 || (strcmp(argv[1], "pilot") != 0 && strcmp(argv[1], "final") != 0)) {
        fprintf(stderr, "usage: %s pilot|final\n", argv[0]);
        return EXIT_FAILURE;
    }

    pilotMode = strcmp(argv[1], "pilot") == 0;
    trials = pilotMode ? 10 : 30;
    runtimePath = pilotMode ? "report/results/pilot_runtime.csv" : "report/results/runtime.csv";
    statsPath = pilotMode ? "report/results/pilot_stats.csv" : "report/results/stats.csv";

    FILE *runtimeCsv = fopen(runtimePath, "w");
    FILE *statsCsv = fopen(statsPath, "w");
    if (runtimeCsv == NULL || statsCsv == NULL) {
        fprintf(stderr, "could not open benchmark CSV output files\n");
        if (runtimeCsv != NULL) {
            fclose(runtimeCsv);
        }
        if (statsCsv != NULL) {
            fclose(statsCsv);
        }
        return EXIT_FAILURE;
    }

    bool runtimeCorrect = runRuntime(runtimeCsv, trials);
    bool statsCorrect = runStats(statsCsv, trials);
    bool writeFailed = ferror(runtimeCsv) != 0 || ferror(statsCsv) != 0;
    fclose(runtimeCsv);
    fclose(statsCsv);

    if (!runtimeCorrect || !statsCorrect || writeFailed) {
        fprintf(stderr, "benchmark failed: inspect correctness and CSV output\n");
        return EXIT_FAILURE;
    }

    printf("%s benchmark complete: %d trials per condition, master seed=%" PRIu64 "\n",
           pilotMode ? "Pilot" : "Final", trials, MASTER_SEED);
    printf("Runtime CSV: %s\nStats CSV: %s\n", runtimePath, statsPath);
    return EXIT_SUCCESS;
}
