#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Global declaration of mutexes.
// Mutex A will be dynamically initialized as recursive in main() to prevent self-deadlocks.
pthread_mutex_t A;
pthread_mutex_t B = PTHREAD_MUTEX_INITIALIZER;

void processar(int nivel)
{
    if (nivel == 0)
        return;

    // The thread recursively acquires mutex A.
    // Because A is now explicitly configured as a recursive mutex,
    // the calling thread can lock it multiple times without triggering a self-deadlock.
    pthread_mutex_lock(&A);
    usleep(1000);

    if (nivel % 2 == 0)
    {
        // Acquiring B after A naturally follows the designated global locking hierarchy.
        pthread_mutex_lock(&B);
        pthread_mutex_unlock(&B);
    }

    // Recursive call: safe reentrancy is now guaranteed.
    processar(nivel - 1);

    // Each successful lock operation on a recursive mutex must be matched by an unlock operation.
    pthread_mutex_unlock(&A);
}

void *thread1(void *arg)
{
    processar(2);
    return NULL;
}

void *thread2(void *arg)
{
    // FIX: Mitigating the inter-thread deadlock by adhering to a strict global locking sequence.
    // Thread 2 must acquire resource A prior to B, unconditionally mirroring the acquisition order
    // implicitly established within the 'processar' function of thread 1.
    pthread_mutex_lock(&A);
    usleep(1000);
    pthread_mutex_lock(&B);

    // Critical section execution sequence
    printf("Thread 2 executada com seguranca.\n");

    // Relinquishing system resources in the reverse mathematical order of their acquisition
    pthread_mutex_unlock(&B);
    pthread_mutex_unlock(&A);

    return NULL;
}

int main()
{
    // Configuring mutex A with recursive attributes dynamically
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0)
    {
        perror("Falha ao inicializar atributos do mutex");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0)
    {
        perror("Falha ao definir o tipo recursivo");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&A, &attr) != 0)
    {
        perror("Falha ao inicializar o mutex A");
        exit(EXIT_FAILURE);
    }

    pthread_t t1, t2;

    // Dispatching the concurrent execution threads
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);

    // Synchronizing the main process barrier
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Execucao concluida com sucesso, estritamente livre de auto-bloqueios e impasses inter-threads.\n");

    // Reclaiming allocated system resources
    pthread_mutexattr_destroy(&attr);
    pthread_mutex_destroy(&A);
    pthread_mutex_destroy(&B);

    return 0;
}