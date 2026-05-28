#include <pthread.h>
#include <stdio.h>
#include <unistd.h >

// Two mutexes used by threads that may attempt to acquire them in different orders
pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t y = PTHREAD_MUTEX_INITIALIZER;

// Thread 1 locks x and then tries to lock y without waiting indefinitely
void *t1(void *arg)
{
    pthread_mutex_lock(&x);
    sleep(1);

    if (pthread_mutex_trylock(&y) != 0)
    {
        // If y is already held by another thread, release x and abort
        // This is a partial timeout-style approach, but it can lead to inconsistent behavior.
        pthread_mutex_unlock(&x);
        return NULL;
    }

    printf("t1 concluiu \n");
    pthread_mutex_unlock(&y);
    pthread_mutex_unlock(&x);
    return NULL;
}

// Thread 2 locks y first and then locks x
// The opposite lock order can still cause conflicts or a deadlock if t1 does not retry cleanly.
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