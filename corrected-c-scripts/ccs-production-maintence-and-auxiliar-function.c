#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Initialization of global synchronization primitives
pthread_mutex_t fila_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t estat_mutex = PTHREAD_MUTEX_INITIALIZER;

void atualizar_estatisticas()
{
    // The function safely acquires its required lock
    pthread_mutex_lock(&estat_mutex);
    usleep(1000);
    printf("Estatisticas atualizadas com sucesso.\n");
    pthread_mutex_unlock(&estat_mutex);
}

void *produtor(void *arg)
{
    // Acquires the primary resource
    pthread_mutex_lock(&fila_mutex);
    usleep(1000);
    printf("Produtor: Operacoes na fila concluidas.\n");

    // FIX: Early release of the resource.
    // By unlocking the fila_mutex BEFORE calling the auxiliary function,
    // we structurally eliminate the "hold and wait" condition.
    // The thread no longer holds a lock while requesting another.
    pthread_mutex_unlock(&fila_mutex);

    // Calls the auxiliary function in a safe, unencumbered state
    atualizar_estatisticas();

    return NULL;
}

void *manutencao(void *arg)
{
    // The maintenance thread can now safely acquire its locks.
    // Since the producer no longer holds the fila_mutex while holding/requesting
    // the estat_mutex, the circular wait condition is mathematically impossible here.
    pthread_mutex_lock(&estat_mutex);
    usleep(1000);
    pthread_mutex_lock(&fila_mutex);

    printf("Manutencao: Rotina de verificacao executada.\n");

    // Releasing locks in the reverse order of acquisition
    pthread_mutex_unlock(&fila_mutex);
    pthread_mutex_unlock(&estat_mutex);

    return NULL;
}

int main()
{
    pthread_t thread_produtor, thread_manutencao;

    // Dispatching the producer and maintenance threads concurrently
    if (pthread_create(&thread_produtor, NULL, produtor, NULL) != 0)
    {
        perror("Erro ao criar a thread produtora");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread_manutencao, NULL, manutencao, NULL) != 0)
    {
        perror("Erro ao criar a thread de manutencao");
        exit(EXIT_FAILURE);
    }

    // Synchronizing the main execution thread with the concurrent workers
    pthread_join(thread_produtor, NULL);
    pthread_join(thread_manutencao, NULL);

    printf("Execucao integralizada sem registo de intertravamentos.\n");

    // Resource cleanup
    pthread_mutex_destroy(&fila_mutex);
    pthread_mutex_destroy(&estat_mutex);

    return 0;
}