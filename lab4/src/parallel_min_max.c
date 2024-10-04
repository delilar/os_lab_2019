#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>
#include <signal.h>

#include "find_min_max.h"
#include "utils.h"

int* child_pids = NULL;
int pnum = -1;

void kill_all_children(int signum) {
    printf("Timeout reached. Killing all child processes...\n");
    for (int i = 0; i < pnum; i++) {
        if (child_pids[i] != 0) {
            kill(child_pids[i], SIGKILL);
            printf("Killed process with PID: %d\n", child_pids[i]);
        }
    }
    exit(1);
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    pnum = -1;
    bool with_files = false;
    int timeout = -1;

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {{"seed", required_argument, 0, 0},
                                          {"array_size", required_argument, 0, 0},
                                          {"pnum", required_argument, 0, 0},
                                          {"by_files", no_argument, 0, 'f'},
                                          {"timeout", required_argument, 0, 0},
                                          {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        seed = atoi(optarg);
                        break;
                    case 1:
                        array_size = atoi(optarg);
                        break;
                    case 2:
                        pnum = atoi(optarg);
                        break;
                    case 3:
                        with_files = true;
                        break;
                    case 4:
                        timeout = atoi(optarg);
                        break;
                    defalut:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case 'f':
                with_files = true;
                break;
            case '?':
                break;
            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"]\n", argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);
    int active_child_processes = 0;

    child_pids = malloc(sizeof(int) * pnum); // Для хранения PID дочерних процессов
    for (int i = 0; i < pnum; i++) child_pids[i] = 0;

    // Устанавливаем обработчик сигнала SIGALRM
    if (timeout > 0) {
        signal(SIGALRM, kill_all_children);
        alarm(timeout);
    }

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int pipefd[2 * pnum];
    if (!with_files) {
        // Создаем пайпы для каждого процесса
        for (int i = 0; i < pnum; i++) {
            if (pipe(pipefd + 2 * i) == -1) {
                perror("pipe");
                return 1;
            }
        }
    }

    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        if (child_pid >= 0) {
            // Успешный fork
            child_pids[i] = child_pid;
            active_child_processes += 1;

            if (child_pid == 0) {
                // Дочерний процесс
                struct MinMax min_max = GetMinMax(array, i * array_size / pnum, (i + 1) * array_size / pnum);

                if (with_files) {
                    // Используем файлы для записи результатов
                    char filename[256];
                    snprintf(filename, sizeof(filename), "min_max_%d.txt", i);
                    FILE *file = fopen(filename, "w");
                    fprintf(file, "%d %d\n", min_max.min, min_max.max);
                    fclose(file);
                } else {
                    // Используем pipe для передачи данных
                    close(pipefd[2 * i]); // Закрываем чтение
                    write(pipefd[2 * i + 1], &min_max, sizeof(struct MinMax));
                    close(pipefd[2 * i + 1]); // Закрываем запись
                }
                return 0;
            }
        } else {
            printf("Fork failed!\n");
            return 1;
        }
    }

    while (active_child_processes > 0) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);  // Используем WNOHANG для неблокирующего ожидания
        if (pid > 0) {
            active_child_processes--;
        }
    }

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        struct MinMax local_min_max;

        if (with_files) {
            // Читаем из файла
            char filename[256];
            snprintf(filename, sizeof(filename), "min_max_%d.txt", i);
            FILE *file = fopen(filename, "r");
            fscanf(file, "%d %d", &local_min_max.min, &local_min_max.max);
            fclose(file);
        } else {
            // Читаем из пайпа
            close(pipefd[2 * i + 1]); // Закрываем запись
            read(pipefd[2 * i], &local_min_max, sizeof(struct MinMax));
            close(pipefd[2 * i]); // Закрываем чтение
        }

        if (local_min_max.min < min_max.min) min_max.min = local_min_max.min;
        if (local_min_max.max > min_max.max) min_max.max = local_min_max.max;
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);
    free(child_pids);

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    fflush(NULL);
    return 0;
}
