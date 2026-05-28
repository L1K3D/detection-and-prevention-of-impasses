#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t b = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t c = PTHREAD_MUTEX_INITIALIZER;

void *t1(void *arg)
{
    pthread_mutex_lock(&a);
    usleep(1000);
    pthread_mutex_lock(&b);
    usleep(1000);
    pthread_mutex_lock(&c);

    printf("t1 executou \n");

    pthread_mutex_unlock(&c);
    pthread_mutex_unlock(&b);
    pthread_mutex_unlock(&a);
    return NULL;
}

void *t2(void *arg)
{
    pthread_mutex_lock(&c);
    usleep(1000);
    pthread_mutex_lock(&b);

    printf("t2 executou \n");

    pthread_mutex_unlock(&b);
    pthread_mutex_unlock(&c);
    return NULL;
}

void *t3(void *arg)
{
    pthread_mutex_lock(&b);
    usleep(1000);
    pthread_mutex_lock(&a);

    printf("t3 executou \n");

    pthread_mutex_unlock(&a);
    pthread_mutex_unlock(&b);
    return NULL;
}