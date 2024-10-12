#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
long long result = 1;

struct FactorialArgs {
    int start;
    int end;
    int mod;
};

void* ThreadFactorial(void* args) {
    struct FactorialArgs* factorialArgs = (struct FactorialArgs*)args;
    long long partial_result = 1;

    for (int i = factorialArgs->start; i <= factorialArgs->end; i++) {
        partial_result = (partial_result * i) % factorialArgs->mod;
    }

    pthread_mutex_lock(&mut);
    result = (result * partial_result) % factorialArgs->mod;
    pthread_mutex_unlock(&mut);

    return NULL;
}

int main(int argc, char** argv) {
    int k = 0, pnum = 0, mod = 0;

    // Парсинг аргументов командной строки
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            k = atoi(argv[++i]);
        } else if (strncmp(argv[i], "--pnum=", 7) == 0) {
            pnum = atoi(&argv[i][7]);
        } else if (strncmp(argv[i], "--mod=", 6) == 0) {
            mod = atoi(&argv[i][6]);
        }
    }

    // Проверка корректности входных данных
    if (k <= 0 || pnum <= 0 || mod <= 0) {
        printf("Usage: %s -k num --pnum=threads --mod=num\n", argv[0]);
        return 1;
    }

    pthread_t threads[pnum];
    struct FactorialArgs args[pnum];

    int range = k / pnum;
    int remainder = k % pnum;

    // Создаем потоки для параллельного вычисления факториала
    int start = 1;
    for (int i = 0; i < pnum; i++) {
        args[i].start = start;
        args[i].end = start + range - 1;
        if (i == pnum - 1) {
            args[i].end += remainder;  // Последний поток берет оставшуюся работу
        }
        args[i].mod = mod;
        start = args[i].end + 1;

        if (pthread_create(&threads[i], NULL, ThreadFactorial, &args[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    // Ожидаем завершения всех потоков
    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Factorial of %d mod %d is: %lld\n", k, mod, result);
    return 0;
}
