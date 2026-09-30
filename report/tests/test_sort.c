/* 정렬 유닛 테스트 — 외부 프레임워크 없이 표준 C만 사용한다. */
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "sort.h"
#include "merge_sort.h"
#include "shell_sort.h"
#include "library_sort.h"

static int checks = 0;
static int failures = 0;

static void printArray(const char *label, const int a[], int n) {
    printf("      %s:", label);
    for (int i = 0; i < n; i++) {
        printf(" %d", a[i]);
    }
    printf("\n");
}

typedef void (*SortFunction)(int[], int);

/* 입력 복사본을 정렬해 기대값과 비교한다. */
static void expectSorted(const char *name, SortFunction sort, const int input[],
                         const int want[], int n) {
    int actual[n > 0 ? n : 1];
    if (n > 0) {
        memcpy(actual, input, (size_t)n * sizeof(int));
    }
    checks++;
    sort(actual, n);
    if (n > 0 && memcmp(actual, want, (size_t)n * sizeof(int)) != 0) {
        failures++;
        printf("FAIL  %s\n", name);
        printArray("got ", actual, n);
        printArray("want", want, n);
        return;
    }
    printf("ok    %s\n", name);
}

static void expectAllSorted(const char *caseName, const int input[], const int want[], int n) {
    static const SortFunction sorts[] = {bubbleSort, shellSort, mergeSort, librarySort};
    static const char *sortNames[] = {
        "Bubble Sort", "Shell Sort", "Merge Sort", "Library Sort"
    };
    char name[96];

    for (size_t index = 0; index < sizeof(sorts) / sizeof(sorts[0]); index++) {
        snprintf(name, sizeof(name), "%s (%s)", caseName, sortNames[index]);
        expectSorted(name, sorts[index], input, want, n);
    }
}

int main(void) {
    {
        const int a[] = {6, 8, 5, 9, 10, 1, 7, 2, 4, 3};
        const int want[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        expectAllSorted("섞인 배열", a, want, 10);
    }
    {
        const int a[] = {1, 2, 3, 4, 5};
        const int want[] = {1, 2, 3, 4, 5};
        expectAllSorted("이미 정렬된 배열", a, want, 5);
    }
    {
        const int a[] = {5, 4, 3, 2, 1};
        const int want[] = {1, 2, 3, 4, 5};
        expectAllSorted("역순 배열", a, want, 5);
    }
    {
        const int a[] = {3, 1, 3, 1, 2};
        const int want[] = {1, 1, 2, 3, 3};
        expectAllSorted("중복이 있는 배열", a, want, 5);
    }
    {
        const int a[] = {7, 7, 7, 7, 7, 7};
        const int want[] = {7, 7, 7, 7, 7, 7};
        expectAllSorted("모든 값이 같은 배열", a, want, 6);
    }
    {
        const int a[] = {-5, -1, -3, 0, -2};
        const int want[] = {-5, -3, -2, -1, 0};
        expectAllSorted("음수와 0", a, want, 5);
    }
    {
        const int a[] = {INT_MAX, 0, INT_MIN, INT_MAX, INT_MIN};
        const int want[] = {INT_MIN, INT_MIN, 0, INT_MAX, INT_MAX};
        expectAllSorted("정수 최솟값과 최댓값", a, want, 5);
    }
    {
        const int a[] = {42};
        const int want[] = {42};
        expectAllSorted("원소 하나", a, want, 1);
    }
    {
        const int a[1] = {0};
        const int want[1] = {0};
        expectAllSorted("빈 배열", a, want, 0);
    }
    {
        int shellValues[] = {4, 1, 3, 1, 2};
        int mergeValues[] = {4, 1, 3, 1, 2};
        const int want[] = {1, 1, 2, 3, 4};
        SortStats stats;

        shellSortWithStats(shellValues, 5, &stats);
        checks++;
        if (memcmp(shellValues, want, sizeof(want)) != 0 || stats.comparisons == 0 ||
            stats.moves == 0 || stats.auxiliary_bytes != sizeof(int)) {
            failures++;
            printf("FAIL  Shell Sort 통계\n");
        } else {
            printf("ok    Shell Sort 통계\n");
        }

        checks++;
        if (!mergeSortWithStats(mergeValues, 5, &stats)) {
            failures++;
            printf("FAIL  Merge Sort 통계 호출\n");
        } else if (memcmp(mergeValues, want, sizeof(want)) != 0 || stats.comparisons == 0 ||
                   stats.moves == 0 || stats.auxiliary_bytes != sizeof(mergeValues)) {
            failures++;
            printf("FAIL  Merge Sort 통계\n");
        } else {
            printf("ok    Merge Sort 통계\n");
        }
    }
    {
        SortStats stats;
        checks++;
        if (!mergeSortWithStats(NULL, 0, &stats) || stats.comparisons != 0 ||
            stats.moves != 0 || stats.auxiliary_bytes != 0) {
            failures++;
            printf("FAIL  빈 Merge Sort 통계\n");
        } else {
            printf("ok    빈 Merge Sort 통계\n");
        }
    }
    {
        int values[] = {4, -2, 4, 0, INT_MIN, INT_MAX};
        const int want[] = {INT_MIN, -2, 0, 4, 4, INT_MAX};
        SortStats stats;
        checks++;
        if (!librarySortWithStats(values, 6, &stats) ||
            memcmp(values, want, sizeof(want)) != 0 || stats.comparisons == 0 ||
            stats.moves == 0 || stats.auxiliary_bytes == 0) {
            failures++;
            printf("FAIL  Library Sort 통계\n");
        } else {
            printf("ok    Library Sort 통계\n");
        }

        checks++;
        if (!librarySortWithStats(NULL, 0, &stats) || stats.comparisons != 0 ||
            stats.moves != 0 || stats.auxiliary_bytes != 0) {
            failures++;
            printf("FAIL  빈 Library Sort 통계\n");
        } else {
            printf("ok    빈 Library Sort 통계\n");
        }

        checks++;
        if (librarySortWithStats(NULL, 1, &stats)) {
            failures++;
            printf("FAIL  잘못된 Library Sort 입력\n");
        } else {
            printf("ok    잘못된 Library Sort 입력\n");
        }
    }
    {
        static const int sizes[] = {0, 1, 2, 3, 5, 8, 16, 31, 64, 127, 256};
        int input[256];
        int expected[256];
        uint32_t state = UINT32_C(0x5EED1234);

        for (size_t sizeIndex = 0; sizeIndex < sizeof(sizes) / sizeof(sizes[0]); sizeIndex++) {
            int count = sizes[sizeIndex];
            for (int index = 0; index < count; index++) {
                state = state * UINT32_C(1664525) + UINT32_C(1013904223);
                input[index] = (int)(state % UINT32_C(2001)) - 1000;
            }
            if (count > 0) {
                memcpy(expected, input, (size_t)count * sizeof(int));
            }
            bubbleSort(expected, count);
            expectSorted("고정 seed 난수 (Library Sort)", librarySort,
                         input, expected, count);
        }
    }

    printf("\n%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
