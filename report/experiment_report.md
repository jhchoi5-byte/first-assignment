# 정렬 알고리즘의 입력 특성 및 크기에 따른 성능 비교

## — Shell Sort, Merge Sort, Library Sort를 중심으로

## 초록

본 보고서는 Shell Sort, Merge Sort, Library Sort의 성능이 입력 크기와 입력 분포에 따라 어떻게 달라지는지 비교한다. 각 조건은 30회 반복했으며, 세 알고리즘에는 같은 원본 입력의 독립 복사본을 전달했다. Runtime의 주 분석값은 median이고, Q1–Q3 범위를 함께 확인했다. 정렬 결과는 독립 oracle과 원소별로 비교했다.

최대 입력 크기인 20,000개에서 Shell Sort와 Merge Sort는 이 실행 환경에서 대체로 수 밀리초 이내에 완료됐다. 반면 Library Sort는 분포에 따라 median이 약 41.8 ms에서 739.9 ms까지 나타났다. 이는 본 프로젝트의 Library Sort가 정통 randomized Library Sort의 성능을 재현하려는 구현이 아니라, sparse array의 빈 공간과 재배치 원리를 보여주는 deterministic educational implementation이기 때문이다.

## 1. 연구 질문

1. 입력 크기가 커질 때 각 정렬 알고리즘의 관측 실행시간은 어떻게 증가하는가?
2. 이미 정렬된 입력, 역순 입력, 임의 입력, 중복값이 많은 입력이 결과에 어떤 차이를 만드는가?
3. 비교 횟수, 데이터 이동 횟수, 보조 메모리 사용량은 알고리즘별로 어떻게 달라지는가?

## 2. 알고리즘 구현

### Shell Sort

Shell Sort는 gap을 줄여 가며 gap 삽입 정렬을 수행한다. 이 프로젝트는 gap을 정수 나눗셈으로 절반씩 줄이는 sequence를 사용한다. 입력 내부에서 정렬하므로 추가 데이터 배열을 할당하지 않는다.

### Merge Sort

Merge Sort는 구간의 폭을 두 배씩 키우며 인접한 정렬 구간을 병합하는 bottom-up 구현이다. 병합을 위해 입력 크기에 비례하는 임시 배열을 할당하고 해제한다.

### Library Sort

Library Sort는 `2*n`개의 sparse slot에 `value`와 `occupied` 상태를 저장한다. 삽입 위치와 가까운 빈 slot을 선형 탐색하고, 빈 공간까지 원소를 이동해 삽입한다. 삽입을 `1, 2, 4, 8, ...` 크기의 round로 나누고, round 후에는 정렬 순서를 보존하며 gap을 재분배한다.

이 구현은 원리 학습을 위한 deterministic educational variant다. 선형 탐색과 shift/rebalance 비용이 결과에 영향을 주므로 randomized Library Sort의 expected $O(n \log n)$ 성능을 대표한다고 해석하지 않는다.

## 3. 실험 방법

### 입력 조건

| 항목 | 조건 |
|---|---|
| 입력 크기 | 500, 1,000, 2,000, 5,000, 10,000, 20,000 |
| 분포 | random, sorted, reverse, duplicate-heavy |
| 반복 | 각 크기·분포·알고리즘 조합당 final 30 trials |
| Master seed | `20260930` |
| Timer | `clock_gettime(CLOCK_MONOTONIC, ...)` |

`random` 값은 -1,000,000부터 1,000,000 범위에서 생성했다. `sorted` 입력은 고정 seed로 작은 jitter를 더해 만든 비감소 수열이며 `reverse`는 역순 수열이다. `duplicate-heavy` 입력은 -10부터 10의 작은 값 범위에서 생성했다. 각 `(size, distribution, trial)`에 대한 seed를 세 알고리즘이 공유해 같은 원본 입력을 받도록 했다.

각 조건의 알고리즘 실행 순서는 여섯 permutation을 순환한다. Final 30 trials에서는 permutation마다 정확히 5회씩 사용하고, 각 알고리즘이 first, second, third 위치에 각각 10회씩 오도록 했다.

### 측정 구간과 정확성

Runtime timer에는 정렬 함수 호출만 포함했다. 입력 생성, oracle 생성, 배열 복사, 결과 비교, CSV 출력은 timer 밖에서 수행했다. Merge Sort와 Library Sort 함수 내부의 allocation/free는 실제 함수 호출 일부이므로 timer 안에 포함된다.

`comparisons`, `moves`, `auxiliary_bytes`는 별도 Stats run에서 수집했으며 Stats run 실행시간은 runtime 분석에 사용하지 않았다. `qsort`는 correctness oracle 생성에만 사용했다. 세 알고리즘의 각 결과를 oracle과 원소별로 비교했다.

## 4. Runtime 결과

아래 값은 30 trials의 median과 Q1–Q3이며 단위는 ms다.

| 알고리즘 | 분포 | Median | Q1–Q3 |
|---|---|---:|---:|
| Shell Sort | random | 1.681 | 1.666–1.743 |
| Shell Sort | sorted | 0.241 | 0.241–0.251 |
| Shell Sort | reverse | 0.337 | 0.323–0.345 |
| Shell Sort | duplicate-heavy | 0.910 | 0.901–0.921 |
| Merge Sort | random | 1.322 | 1.314–1.347 |
| Merge Sort | sorted | 0.391 | 0.381–0.407 |
| Merge Sort | reverse | 0.285 | 0.276–0.292 |
| Merge Sort | duplicate-heavy | 0.853 | 0.846–0.873 |
| Library Sort | random | 432.218 | 418.876–464.143 |
| Library Sort | sorted | 739.906 | 696.200–771.984 |
| Library Sort | reverse | 41.803 | 41.389–43.930 |
| Library Sort | duplicate-heavy | 308.167 | 288.608–330.893 |

Shell Sort는 이 실행에서 sorted 입력에서 가장 빨랐고, Merge Sort는 reverse 입력에서 가장 빨랐다. Library Sort는 reverse 입력의 median이 다른 세 분포보다 낮았지만, random·sorted·duplicate-heavy에서는 훨씬 긴 시간을 보였다. 이 패턴은 이번 Library Sort 구현의 선형 삽입 경계 탐색과 입력별 shift/rebalance 비용에 부합한다.

### 크기 증가에 따른 관측 비율

아래 비율은 각 그룹에서 `n=20,000` median을 `n=500` median으로 나눈 값이다. **관측된 scaling ratio**이며 이론적 복잡도 추정은 아니다.

| 알고리즘 | random | sorted | reverse | duplicate-heavy |
|---|---:|---:|---:|---:|
| Shell Sort | 66.891x | 71.802x | 64.958x | 55.444x |
| Merge Sort | 62.066x | 63.214x | 57.255x | 46.820x |
| Library Sort | 1733.411x | 1606.344x | 826.149x | 1372.010x |

Shell Sort와 Merge Sort의 median runtime은 이 크기 범위에서 약 47–72배 증가했다. Library Sort는 분포에 따라 약 826–1,733배 증가했다. 이 차이는 해당 Library Sort 변형과 측정한 입력 집합에 관한 관측이며, 다른 Library Sort 변형이나 이론적 복잡도를 대변하지 않는다.

## 5. 연산 통계와 보조 메모리

아래는 `n=20,000` Stats run의 median이다.

| 알고리즘 | 분포 | Comparisons | Moves | Auxiliary bytes |
|---|---|---:|---:|---:|
| Shell Sort | random | 617,255.5 | 887,283.0 | 4 |
| Shell Sort | sorted | 260,005.0 | 520,010.0 | 4 |
| Shell Sort | reverse | 375,138.0 | 655,130.0 | 4 |
| Shell Sort | duplicate-heavy | 367,679.0 | 636,664.5 | 4 |
| Merge Sort | random | 267,350.5 | 585,280.0 | 80,000 |
| Merge Sort | sorted | 153,424.0 | 585,280.0 | 80,000 |
| Merge Sort | reverse | 139,216.0 | 585,280.0 | 80,000 |
| Merge Sort | duplicate-heavy | 263,916.0 | 585,280.0 | 80,000 |
| Library Sort | random | 100,248,717.0 | 112,682.5 | 400,000 |
| Library Sort | sorted | 199,990,000.0 | 69,069,604.0 | 400,000 |
| Library Sort | reverse | 19,999.0 | 64,056,467.0 | 400,000 |
| Library Sort | duplicate-heavy | 95,219,682.0 | 913,942.5 | 400,000 |

비교 횟수는 구현별로 정의된 카운터다. 특히 Library Sort의 `comparisons`에는 gap 탐색 중 수행하는 occupied-slot 검사 횟수가 포함되지 않는다. 따라서 이를 세 알고리즘의 동일한 기계 연산량으로 해석하면 안 된다. `moves`도 각 구현에서 기록하는 원소 복사·이동의 정의에 따른다.

`auxiliary_bytes`는 allocator overhead나 process RSS가 아니라 구현이 요청한 보조 메모리 할당량이다. 이 입력 크기에서 Shell Sort는 임시 정수 한 개(4 bytes), Merge Sort는 `int` scratch 배열(80,000 bytes), Library Sort는 sparse slot 배열과 scratch 배열의 합(400,000 bytes)을 기록했다.

## 6. 검증 결과

Final runtime CSV와 Stats CSV는 각각 2,160 data rows를 포함한다. 720개 `(size, distribution, trial)` case마다 세 알고리즘이 정확히 하나씩 존재한다. 같은 case의 세 알고리즘은 동일한 seed를 공유한다. Final의 여섯 permutation은 각 size-distribution 조건에서 각각 5회, 각 알고리즘의 실행 위치는 first/second/third 각각 10회였다.

- Runtime correctness: 2,160 / 2,160 성공
- Stats correctness 및 status: 2,160 / 2,160 성공
- 0 또는 음수 elapsed 행: 0
- `make test`: C 53 checks, 0 failures; Python 7 tests 통과
- Final benchmark 전체 wall-clock: 128.192초

Pilot은 feasibility 확인용 10 trials이며 final 결과와 합산하지 않았다. Final 결과는 `runtime.csv` 및 `stats.csv`에 보존했고, runtime Q1/Q3와 stats 그룹별 요약은 각각의 summary CSV에 기록했다.

## 7. 해석의 한계

1. Library Sort 결과는 이 프로젝트의 deterministic educational implementation에 한정된다. randomized Library Sort의 expected $O(n \log n)$ 결과로 일반화할 수 없다.
2. Library Sort comparisons에서 occupied-slot 탐색 검사가 제외되어 비교 횟수만으로 알고리즘의 총 작업량을 직접 순위화할 수 없다.
3. Auxiliary bytes는 구현이 요청한 할당량으로 allocator metadata, fragmentation, process RSS를 측정하지 않는다.
4. Runtime은 특정 컴파일러 설정과 실행 환경에서 얻은 측정값이다. median 및 quartile은 표본 변동을 요약하지만 다른 환경의 절대 성능을 보장하지 않는다.
5. `n=500`과 `n=20,000`의 비율은 두 측정점의 관측 비교이며 이론적 점근 복잡도를 추정한 값이 아니다.

## 8. 재현 자료와 Figure

- Raw runtime: [`results/runtime.csv`](results/runtime.csv)
- Raw Stats: [`results/stats.csv`](results/stats.csv)
- Runtime summary: [`results/runtime_summary.csv`](results/runtime_summary.csv)
- Stats summary: [`results/stats_summary.csv`](results/stats_summary.csv)
- Benchmark source: [`benchmark.c`](benchmark.c)
- Plotting source: [`plot_results.py`](plot_results.py)
- Runtime figure: [`figures/runtime.png`](figures/runtime.png) · [`figures/runtime.pdf`](figures/runtime.pdf)
- Comparisons figure: [`figures/comparisons.png`](figures/comparisons.png) · [`figures/comparisons.pdf`](figures/comparisons.pdf)
- Moves figure: [`figures/moves.png`](figures/moves.png) · [`figures/moves.pdf`](figures/moves.pdf)
- Auxiliary memory figure: [`figures/auxiliary_memory.png`](figures/auxiliary_memory.png) · [`figures/auxiliary_memory.pdf`](figures/auxiliary_memory.pdf)
