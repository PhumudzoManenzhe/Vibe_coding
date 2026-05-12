#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <stdbool.h>

int partition(int *arr, int low, int high) {
    int pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    int temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return (i + 1);
}

void quicksort_seq(int *arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quicksort_seq(arr, low, pi - 1);
        quicksort_seq(arr, pi + 1, high);
    }
}

void quicksort_sections(int *arr, int low, int high, int threshold) {
    if (low < high) {
        if (high - low < threshold) {
            quicksort_seq(arr, low, high);
        } else {
            int pi = partition(arr, low, high);
            
            #pragma omp parallel sections
            {
                #pragma omp section
                quicksort_sections(arr, low, pi - 1, threshold);
                
                #pragma omp section
                quicksort_sections(arr, pi + 1, high, threshold);
            }
        }
    }
}

void quicksort_tasks(int *arr, int low, int high, int threshold) {
    if (low < high) {
        if (high - low < threshold) {
            quicksort_seq(arr, low, high);
        } else {
            int pi = partition(arr, low, high);
            
            #pragma omp task shared(arr) firstprivate(low, pi, threshold)
            quicksort_tasks(arr, low, pi - 1, threshold);
            
            #pragma omp task shared(arr) firstprivate(high, pi, threshold)
            quicksort_tasks(arr, pi + 1, high, threshold);
            #pragma omp taskwait
        }
    }
}

bool validate(int *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i+1]) {
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <array_size> <threshold>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int threshold = atoi(argv[2]);
    omp_set_max_active_levels(16);

    int *arr_seq = (int *)malloc(n * sizeof(int));
    int *arr_sec = (int *)malloc(n * sizeof(int));
    int *arr_tsk = (int *)malloc(n * sizeof(int));
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        int val = rand() % n;
        arr_seq[i] = val;
        arr_sec[i] = val;
        arr_tsk[i] = val;
    }

    double start, end;
    double time_seq, time_sec, time_tsk;

    start = omp_get_wtime();
    quicksort_seq(arr_seq, 0, n - 1);
    end = omp_get_wtime();
    time_seq = end - start;

    start = omp_get_wtime();
    quicksort_sections(arr_sec, 0, n - 1, threshold);
    end = omp_get_wtime();
    time_sec = end - start;

    start = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        quicksort_tasks(arr_tsk, 0, n - 1, threshold);
    }
    end = omp_get_wtime();
    time_tsk = end - start;

    bool valid_sec = validate(arr_sec, n);
    bool valid_tsk = validate(arr_tsk, n);

    printf("Sections/section parallel sorting: Validate %s.\n", valid_sec ? "passed" : "FAILED");
    printf("Task parallel sorting: Validate %s.\n", valid_tsk ? "passed" : "FAILED");
    
    printf("Sequential time: %f s\n", time_seq);
    printf("SECTIONS/SECTION Parallel time: %f s\n", time_sec);
    printf("TASK Parallel time: %f s\n", time_tsk);
    
    printf("SECTIONS/SECTION Parallel speedup: %f\n", time_seq / time_sec);
    printf("TASK Parallel speedup: %f\n", time_seq / time_tsk);
    printf("Done\n");

    free(arr_seq);
    free(arr_sec);
    free(arr_tsk);

    return 0;
}