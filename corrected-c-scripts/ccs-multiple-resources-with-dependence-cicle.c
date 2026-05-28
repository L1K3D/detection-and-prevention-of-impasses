#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Initialization of the global synchronization primitives
pthread_mutex_t a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t b = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t c = PTHREAD_MUTEX_INITIALIZER;

void *t1(void *arg)
{
    // Thread 1 inherently respects the global hierarchical order: A -> B -> C
    pthread_mutex_lock(&a);
    usleep(1000);
    pthread_mutex_lock(&b);
    usleep(1000);
    pthread_mutex_lock(&c);

    // Critical section execution
    printf("Thread t1 executada com sucesso.\n");

    // Relinquishing resources in the exact mathematical reverse order of their acquisition
    pthread_mutex_unlock(&c);
    pthread_mutex_unlock(&b);
    pthread_mutex_unlock(&a);

    return NULL;
}

void *t2(void *arg)
{
    // FIX: Architectural compliance with the established global order.
    // To prevent the partial dependency cycle with t1, thread 2 MUST acquire
    // mutex 'b' strictly BEFORE mutex 'c'.
    pthread_mutex_lock(&b);
    usleep(1000);
    pthread_mutex_lock(&c);

    // Critical section execution
    printf("Thread t2 executada com sucesso.\n");

    // Releasing locks sequentially backwards
    pthread_mutex_unlock(&c);
    pthread_mutex_unlock(&b);

    return NULL;
}

void *t3(void *arg)
{
    // FIX: Eradicating the direct circular wait condition with t1.
    // Thread 3 MUST acquire mutex 'a' before mutex 'b', adhering to the global state hierarchy.
    pthread_mutex_lock(&a);
    usleep(1000);
    pthread_mutex_lock(&b);

    // Critical section execution
    printf("Thread t3 executada com sucesso.\n");

    // Releasing locks symmetrically
    pthread_mutex_unlock(&b);
    pthread_mutex_unlock(&a);

    return NULL;
}

int main()
{
    pthread_t thread_1, thread_2, thread_3;

    // Dispatching the concurrent execution threads
    if (pthread_create(&thread_1, NULL, t1, NULL) != 0)
    {
        perror("Falha na alocacao da thread t1");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&thread_2, NULL, t2, NULL) != 0)
    {
        perror("Falha na alocacao da thread t2");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&thread_3, NULL, t3, NULL) != 0)
    {
        perror("Falha na alocacao da thread t3");
        exit(EXIT_FAILURE);
    }

    // Synchronizing the main process barrier, ensuring full systemic completion
    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);
    pthread_join(thread_3, NULL);

    printf("Sistema convergido: Todas as transacoes foram executadas sem presenca de ciclos mortos.\n");

    // Graceful destruction of the synchronization objects to prevent memory leaks
    pthread_mutex_destroy(&a);
    pthread_mutex_destroy(&b);
    pthread_mutex_destroy(&c);

    return 0;
}