#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Initialization of the global synchronization primitives
pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t y = PTHREAD_MUTEX_INITIALIZER;

void *t1(void *arg)
{
    // Thread 1 strictly follows the global lock acquisition hierarchy: x then y.
    pthread_mutex_lock(&x);
    sleep(1);

    // The trylock and task abandonment logic has been structurally removed.
    // By relying on the global order, a standard blocking lock is now mathematically safe.
    pthread_mutex_lock(&y);

    printf("Thread t1 concluiu a sua seccao critica com integridade.\n");

    // Relinquishing system resources symmetrically in the reverse order
    pthread_mutex_unlock(&y);
    pthread_mutex_unlock(&x);

    return NULL;
}

void *t2(void *arg)
{
    // FIX: Architectural compliance with the global locking hierarchy.
    // To eradicate any possibility of deadlock without resorting to asymmetrical thread starvation,
    // t2 MUST acquire mutex 'x' BEFORE mutex 'y', mirroring the order established in t1.
    pthread_mutex_lock(&x);
    sleep(1);
    pthread_mutex_lock(&y);

    printf("Thread t2 concluiu a sua seccao critica com integridade.\n");

    // Relinquishing system resources
    pthread_mutex_unlock(&y);
    pthread_mutex_unlock(&x);

    return NULL;
}

int main()
{
    pthread_t thread_1, thread_2;

    // Instantiating the concurrent execution threads
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

    // Main process synchronization barrier ensuring full completion
    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);

    printf("Sistema convergido: Todas as transacoes foram executadas sem abandono de tarefas.\n");

    // Graceful destruction of the synchronization objects
    pthread_mutex_destroy(&x);
    pthread_mutex_destroy(&y);

    return 0;
}