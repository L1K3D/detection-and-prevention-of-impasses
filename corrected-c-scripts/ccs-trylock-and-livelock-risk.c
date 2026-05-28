#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Initialization of the concurrent synchronization objects
pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

void *t1(void *arg)
{
    // Establishing a strict, global lock acquisition hierarchy: m1 is locked prior to m2.
    pthread_mutex_lock(&m1);
    pthread_mutex_lock(&m2);

    // Critical section execution sequence
    printf("t1 executando \n");

    // Relinquishing system resources in the mathematically inverse order of acquisition
    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);

    return NULL;
}

void *t2(void *arg)
{
    // FIX: The livelock vulnerability was intrinsically bound to the non-blocking polling
    // mechanism coupled with inverse resource requests. By forcing t2 to strictly comply
    // with the universal locking order (m1 then m2), we algorithmically neutralize
    // any potential for both livelock regressions and classical deadlocks.
    pthread_mutex_lock(&m1);
    pthread_mutex_lock(&m2);

    // Critical section execution sequence
    printf("t2 executando \n");

    // Relinquishing system resources
    pthread_mutex_unlock(&m2);
    pthread_mutex_unlock(&m1);

    return NULL;
}

int main()
{
    pthread_t thread1, thread2;

    // Instantiating the execution threads
    pthread_create(&thread1, NULL, t1, NULL);
    pthread_create(&thread2, NULL, t2, NULL);

    // Main process synchronization barrier
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Concorrencia executada e estabilizada sem incidentes de impasses ativados ou passivos.\n");

    return 0;
}