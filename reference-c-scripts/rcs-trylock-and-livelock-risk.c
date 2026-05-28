#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Two mutexes used by threads that may retry with trylock
pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

// Thread 1 repeatedly locks m1 and then tries to lock m2
void *t1(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&m1);
        if (pthread_mutex_trylock(&m2) == 0)
        {
            break;
        }
        pthread_mutex_unlock(&m1);
        usleep(100);
    }

    printf("t1 executando \n");
    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);
    return NULL;
}

// Thread 2 repeatedly locks m2 and then tries to lock m1
void *t2(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&m2);
        if (pthread_mutex_trylock(&m1) == 0)
        {
            break;
        }
        pthread_mutex_unlock(&m2);
        usleep(100);
    }

    printf("t2 executando \n");
    pthread_mutex_unlock(&m1);
    pthread_mutex_unlock(&m2);
    return NULL;
}