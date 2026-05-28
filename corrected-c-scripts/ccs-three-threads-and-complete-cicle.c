#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Initialize mutexes representing the three shared resources
pthread_mutex_t r1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t r2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t r3 = PTHREAD_MUTEX_INITIALIZER;

void *t1(void *arg)
{
    // Thread 1 acquires resources following the global order: r1 then r2
    pthread_mutex_lock(&r1);
    sleep(1);
    pthread_mutex_lock(&r2);

    printf("t1 executou \n");

    // Unlocking resources in the reverse order of acquisition
    pthread_mutex_unlock(&r2);
    pthread_mutex_unlock(&r1);

    return NULL;
}

void *t2(void *arg)
{
    // Thread 2 acquires resources following the global order: r2 then r3
    pthread_mutex_lock(&r2);
    sleep(1);
    pthread_mutex_lock(&r3);

    printf("t2 executou \n");

    pthread_mutex_unlock(&r3);
    pthread_mutex_unlock(&r2);

    return NULL;
}

void *t3(void *arg)
{
    // FIX: Thread 3 must also strictly adhere to the global lock ordering.
    // Instead of locking r3 then r1, it MUST lock r1 before r3.
    // This breaks the circular wait condition and mathematically prevents deadlocks.
    pthread_mutex_lock(&r1);

    // The delay simulates concurrent execution overlap, but the system is now structurally safe
    sleep(1);

    pthread_mutex_lock(&r3);

    printf("t3 executou \n");

    // Best practice: unlock in the exact reverse order of acquisition
    pthread_mutex_unlock(&r3);
    pthread_mutex_unlock(&r1);

    return NULL;
}

int main()
{
    pthread_t thread1, thread2, thread3;

    // Instantiate and launch the three threads concurrently
    pthread_create(&thread1, NULL, t1, NULL);
    pthread_create(&thread2, NULL, t2, NULL);
    pthread_create(&thread3, NULL, t3, NULL);

    // Synchronize the main process, waiting for all threads to successfully complete
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    printf("Todas as execucoes foram concluidas sem impasses!\n");

    return 0;
}