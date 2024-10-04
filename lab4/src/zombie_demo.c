#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        // Ошибка при вызове fork()
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        // Дочерний процесс
        printf("Child process (PID: %d) is running...\n", getpid());
        sleep(2);  // Задержка для эмуляции работы процесса
        printf("Child process (PID: %d) is exiting...\n", getpid());
        exit(0);   // Дочерний процесс завершается
    } else {
        // Родительский процесс
        printf("Parent process (PID: %d) is running...\n", getpid());
        printf("Child process (PID: %d) has been created.\n", pid);

        // Родитель не вызывает wait(), и дочерний процесс становится зомби
        //wait(NULL);
        printf("Parent process is sleeping, child process will become zombie...\n");
        sleep(10);  // Достаточно времени для появления зомби-процесса

        // Для проверки: вызовите команду "ps aux" в другом терминале, чтобы увидеть зомби-процесс.

        printf("Parent process is exiting...\n");
    }

    return 0;
}
