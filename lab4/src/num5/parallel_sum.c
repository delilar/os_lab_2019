#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "sum_lib.h"
#include "utils.h"

void *ThreadSum(void *args) {
    struct SumArgs *sum_args = (struct SumArgs *)args;
    return (void *)(size_t)Sum(sum_args);
}

int main(int argc, char **argv) {
    if (argc != 7) {
        printf("Usage: %s --threads_num <num> --seed <num> --array_size <num>\n", argv[0]);
        return 1;
    }

    uint32_t threads_num = atoi(argv[2]);
    uint32_t seed = atoi(argv[4]);
    uint32_t array_size = atoi(argv[6]);
    
    pthread_t threads[threads_num];
    int *array = malloc(sizeof(int) * array_size);

    // Генерация массива
    GenerateArray(array, array_size, seed);

    struct SumArgs args[threads_num];
    int chunk_size = array_size / threads_num;

    // Измерение времени
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Создание потоков
    for (uint32_t i = 0; i < threads_num; i++) {
        args[i].array = array;
        args[i].begin = i * chunk_size;
        args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * chunk_size;

        if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
            printf("Error: pthread_create failed!\n");
            return 1;
        }
    }

    // Сбор результатов
    int total_sum = 0;
    for (uint32_t i = 0; i < threads_num; i++) {
        int sum = 0;
        pthread_join(threads[i], (void **)&sum);
        total_sum += sum;
    }

    // Измерение времени окончания
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    // Подсчет времени
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                          (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    free(array);

    printf("Total sum: %d\n", total_sum);
    printf("Elapsed time: %f seconds\n", elapsed_time);
    return 0;
}
