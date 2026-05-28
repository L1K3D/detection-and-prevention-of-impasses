#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t impressora = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t spooler = PTHREAD_MUTEX_INITIALIZER;

void *usuario1(void *arg)
{
    pthread_mutex_lock(&impressora);
    usleep(1000);
    pthread_mutex_lock(&spooler);

    printf(" usuario1 imprimindo \n");

    pthread_mutex_unlock(&spooler);
    pthread_mutex_unlock(&impressora);
    return NULL;
}

void *usuario2(void *arg)
{
    pthread_mutex_lock(&spooler);
    usleep(1000);
    pthread_mutex_lock(&impressora);

    printf(" usuario2 imprimindo \n");

    pthread_mutex_unlock(&impressora);
    pthread_mutex_unlock(&spooler);
    return NULL;
}