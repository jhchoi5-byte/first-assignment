/* Shell Sort 인터페이스. */
#ifndef SHELL_SORT_H
#define SHELL_SORT_H

#include "sort_stats.h"

/* a[0..n-1]을 제자리에서 오름차순으로 정렬한다. */
void shellSort(int a[], int n);

/* stats가 NULL이 아니면 비교·이동·보조 공간 사용량을 기록한다. */
void shellSortWithStats(int a[], int n, SortStats *stats);

#endif /* SHELL_SORT_H */
