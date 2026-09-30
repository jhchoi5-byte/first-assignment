/* Merge Sort 인터페이스. */
#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include "sort_stats.h"

/* a[0..n-1]을 오름차순으로 정렬한다. */
void mergeSort(int a[], int n);

/* 성공하면 1, 잘못된 입력 또는 임시 메모리 할당 실패 시 0을 반환한다. */
int mergeSortWithStats(int a[], int n, SortStats *stats);

#endif /* MERGE_SORT_H */
