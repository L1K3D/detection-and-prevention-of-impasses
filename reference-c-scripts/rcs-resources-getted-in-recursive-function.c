#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Mutex A and mutex B protect different shared resources
pthread_mutex_t A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t B = PTHREAD_MUTEX_INITIALIZER;

// Recursive processing that may hold A across nested calls and lock B conditionally
void processar(int nivel)
{
    if (nivel == 0)
        return;

    pthread_mutex_lock(&A);
    usleep(1000);

    if (nivel % 2 == 0)
    {
        pthread_mutex_lock(&B);
        pthread_mutex_unlock(&B);
    }

    processar(nivel - 1);
    pthread_mutex_unlock(&A);
}

void *thread1(void *arg)
{
    processar(2);
    return NULL;
}

// Thread 2 locks B first, then attempts to lock A. This can conflict with recursive processing.
void *thread2(void *arg)
{
    pthread_mutex_lock(&B);
    usleep(1000);
    pthread_mutex_lock(&A);

    pthread_mutex_unlock(&A);

    pthread_mutex_unlock(&B);
    return NULL;
}