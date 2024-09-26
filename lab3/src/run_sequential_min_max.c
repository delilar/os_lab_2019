#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s seed arraysize\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        printf("Fork failed!\n");
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс: выполняем sequential_min_max
        printf("Running sequential_min_max...\n");
        execl("./sequential_min_max", "./sequential_min_max", argv[1], argv[2], NULL);
        // Если exec завершился неудачей, выводим сообщение об ошибке
        perror("exec failed");
        return 1;
    } else {
        // Родительский процесс: ждем завершения дочернего процесса
        int status;
        waitpid(pid, &status, 0);
        printf("sequential_min_max finished with status: %d\n", WEXITSTATUS(status));
    }

    return 0;
}
