#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread_func1(void* arg) {
    // Захватываем первый мьютекс
    pthread_mutex_lock(&mutex1);
    printf("Thread 1 locked mutex1\n");
    
    sleep(1);  // Задержка для эмуляции работы

    // Пытаемся захватить второй мьютекс
    printf("Thread 1 trying to lock mutex2\n");
    pthread_mutex_lock(&mutex2);
    printf("Thread 1 locked mutex2\n");

    // Освобождаем оба мьютекса
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);

    return NULL;
}

void* thread_func2(void* arg) {
    // Захватываем второй мьютекс
    pthread_mutex_lock(&mutex2);
    printf("Thread 2 locked mutex2\n");
    
    sleep(1);  // Задержка для эмуляции работы

    // Пытаемся захватить первый мьютекс
    printf("Thread 2 trying to lock mutex1\n");
    pthread_mutex_lock(&mutex1);
    printf("Thread 2 locked mutex1\n");

    // Освобождаем оба мьютекса
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);

    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Создаем два потока
    pthread_create(&thread1, NULL, thread_func1, NULL);
    pthread_create(&thread2, NULL, thread_func2, NULL);

    // Ожидаем завершения потоков
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Both threads finished\n");

    return 0;
}
