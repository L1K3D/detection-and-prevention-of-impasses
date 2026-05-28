#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Two account locks represent two shared resources in a transfer scenario
pthread_mutex_t conta1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t conta2 = PTHREAD_MUTEX_INITIALIZER;

// Transfer 1 locks account 1 first, then account 2
void *transferencia_1(void *arg)
{
    pthread_mutex_lock(&conta1);
    sleep(1);
    pthread_mutex_lock(&conta2);
    printf(" Transferencia 1 concluida \n");
    pthread_mutex_unlock(&conta2);
    pthread_mutex_unlock(&conta1);
    return NULL;
}

// Transfer 2 locks account 2 first, then account 1
// This reverse order creates a classic deadlock risk if both threads run concurrently.
void *transferencia_2(void *arg)
{
    pthread_mutex_lock(&conta2);
    sleep(1);
    pthread_mutex_lock(&conta1);

    printf(" Transferencia 2 concluida \n");

    pthread_mutex_unlock(&conta1);
    pthread_mutex_unlock(&conta2);
    return NULL;
}