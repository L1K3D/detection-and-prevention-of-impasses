#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Initialization of the global synchronization primitives
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t banco_mutex = PTHREAD_MUTEX_INITIALIZER;

void registrar_log()
{
    // Safely acquires the log_mutex.
    // In the corrected architecture, the calling thread does not hold it beforehand.
    pthread_mutex_lock(&log_mutex);
    usleep(1000);
    printf("Log registrado com exito.\n");
    pthread_mutex_unlock(&log_mutex);
}

void atualizar_banco()
{
    // Acquires the database mutex to protect the database critical section
    pthread_mutex_lock(&banco_mutex);
    usleep(1000);
    printf("Banco de dados atualizado.\n");

    // It is mathematically safe to invoke registrar_log() here because
    // the caller function (thread1) no longer holds log_mutex, thereby
    // neutralizing the self-deadlock vulnerability.
    registrar_log();

    pthread_mutex_unlock(&banco_mutex);
}

void *thread1(void *arg)
{
    pthread_mutex_lock(&log_mutex);
    usleep(1000);
    printf("Thread 1: Executando procedimento primario de log.\n");

    // FIX: The core vulnerability was maintaining the possession of log_mutex
    // while executing atualizar_banco(). By releasing the lock strictly BEFORE
    // the nested function call, we intrinsically prevent the self-deadlock.
    pthread_mutex_unlock(&log_mutex);

    atualizar_banco();

    return NULL;
}

// Proposed second thread to demonstrate safe concurrent execution
void *thread2(void *arg)
{
    // Thread 2 attempts to perform the database update directly.
    // Due to the structural fix in thread1, there is no circular wait condition.
    printf("Thread 2: Iniciando operacao independente no banco.\n");
    atualizar_banco();

    return NULL;
}

int main()
{
    pthread_t t1, t2;

    // Instantiating and dispatching the threads concurrently
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);

    // Synchronizing the main process execution with the completion of the threads
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Todas as execucoes foram concluidas de forma integra e sem intertravamentos.\n");

    return 0;
}