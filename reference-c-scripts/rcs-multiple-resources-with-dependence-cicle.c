#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Three shared resources with different lock acquisition orders
pthread_mutex_t a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t b = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t c = PTHREAD_MUTEX_INITIALIZER;

// Thread 1 locks a, then b, then c
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

// Thread 2 locks c, then b
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

// Thread 3 locks b, then a
// Together, the lock order in t1, t2, and t3 can form a circular dependency.
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