/* 정렬 통계. auxiliary_bytes는 최대 보조 원소 저장 공간이다. */
#ifndef SORT_STATS_H
#define SORT_STATS_H

#include <stddef.h>

typedef struct {
    size_t comparisons;
    size_t moves;
    size_t auxiliary_bytes;
} SortStats;

#endif /* SORT_STATS_H */
