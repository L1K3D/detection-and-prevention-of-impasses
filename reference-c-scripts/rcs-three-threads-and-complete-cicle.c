#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Three resource locks used by three threads in a circular acquisition pattern
pthread_mutex_t r1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t r2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t r3 = PTHREAD_MUTEX_INITIALIZER;

// Thread 1 locks r1, then r2
void *t1(void *arg)
{
    pthread_mutex_lock(&r1);
    sleep(1);
    pthread_mutex_lock(&r2);
    printf("t1 executou \n");
    pthread_mutex_unlock(&r2);
    pthread_mutex_unlock(&r1);
    return NULL;
}

// Thread 2 locks r2, then r3
void *t2(void *arg)
{
    pthread_mutex_lock(&r2);
    sleep(1);

    pthread_mutex_lock(&r3);
    printf("t2 executou \n");
    pthread_mutex_unlock(&r3);
    pthread_mutex_unlock(&r2);
    return NULL;
}

// Thread 3 locks r3, then r1
// The combination of these acquisition orders forms a circular dependency across threads.
void *t3(void *arg)
{
    pthread_mutex_lock(&r3);
    sleep(1);
    pthread_mutex_lock(&r1);
    printf("t3 executou \n");
    pthread_mutex_unlock(&r1);
    pthread_mutex_unlock(&r3);
    return NULL;
}