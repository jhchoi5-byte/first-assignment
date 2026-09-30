#ifndef LIBRARY_SORT_H
#define LIBRARY_SORT_H

#include "sort_stats.h"

/* a[0..n-1]을 제자리에서 오름차순으로 정렬한다. */
void librarySort(int a[], int n);

/* 성공하면 1, 잘못된 입력 또는 메모리 할당 실패 시 0을 반환한다. */
int librarySortWithStats(int a[], int n, SortStats *stats);

#endif /* LIBRARY_SORT_H */
