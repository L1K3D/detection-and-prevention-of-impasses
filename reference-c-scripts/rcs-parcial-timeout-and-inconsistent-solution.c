#include <pthread.h>
#include <stdio.h>
#include <unistd.h >

pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t y = PTHREAD_MUTEX_INITIALIZER;

void *t1(void *arg)
{
    pthread_mutex_lock(&x);
    sleep(1);

    if (pthread_mutex_trylock(&y) != 0)
    {
        pthread_mutex_unlock(&x);
        return NULL;
    }

    printf("t1 concluiu \n");
    pthread_mutex_unlock(&y);
    pthread_mutex_unlock(&x);
    return NULL;
}

void *t2(void *arg)
{
    pthread_mutex_lock(&y);
    sleep(1);
    pthread_mutex_lock(&x);

    printf("t2 concluiu \n");
    pthread_mutex_unlock(&x);
    pthread_mutex_unlock(&y);
    return NULL;
}