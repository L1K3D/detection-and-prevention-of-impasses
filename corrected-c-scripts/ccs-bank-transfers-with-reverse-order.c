#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Initialize mutexes for the two bank accounts
pthread_mutex_t conta1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t conta2 = PTHREAD_MUTEX_INITIALIZER;

void *transferencia_1(void *arg)
{
    // Thread 1 acquires locks following the global lock order: conta1 first, then conta2.
    pthread_mutex_lock(&conta1);

    // Simulating some processing or network delay
    sleep(1);

    pthread_mutex_lock(&conta2);

    // Critical section: both resources are safely acquired
    printf(" Transferencia 1 concluida \n");

    // Unlocking resources. Doing it in the reverse order of acquisition is a best practice.
    pthread_mutex_unlock(&conta2);
    pthread_mutex_unlock(&conta1);

    return NULL;
}

void *transferencia_2(void *arg)
{
    // FIX: Enforcing global lock order to prevent circular wait (Deadlock).
    // Regardless of the transfer direction, we MUST acquire conta1 BEFORE conta2.
    pthread_mutex_lock(&conta1);

    // Even with a delay here, a deadlock is now mathematically impossible
    // because thread 1 would be waiting for conta1, not holding it.
    sleep(1);

    pthread_mutex_lock(&conta2);

    // Critical section
    printf(" Transferencia 2 concluida \n");

    // Unlocking resources safely
    pthread_mutex_unlock(&conta2);
    pthread_mutex_unlock(&conta1);
    
    return NULL;
}

int main()
{
    pthread_t t1, t2;

    // Create the two threads to simulate concurrent bank transfers
    pthread_create(&t1, NULL, transferencia_1, NULL);
    pthread_create(&t2, NULL, transferencia_2, NULL);

    // Wait for both threads to finish execution before exiting the main program
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf(" Todas as transferencias foram concluidas com sucesso! \n");

    return 0;
}