#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Mutex for protecting the work queue
pthread_mutex_t fila_mutex = PTHREAD_MUTEX_INITIALIZER;
// Mutex for protecting statistics updates
pthread_mutex_t estat_mutex = PTHREAD_MUTEX_INITIALIZER;

// Update statistics while holding the statistics mutex
void atualizar_estatisticas()
{
    pthread_mutex_lock(&estat_mutex);
    usleep(1000);
    pthread_mutex_unlock(&estat_mutex);
}

// Producer locks the queue first, then updates statistics indirectly
void *produtor(void *arg)
{
    pthread_mutex_lock(&fila_mutex);
    usleep(1000);
    atualizar_estatisticas();
    pthread_mutex_unlock(&fila_mutex);
    return NULL;
}

// Maintenance locks statistics first, then the queue, creating an opposite lock order.
void *manutencao(void *arg)
{
    pthread_mutex_lock(&estat_mutex);
    usleep(1000);
    pthread_mutex_lock(&fila_mutex);

    pthread_mutex_unlock(&fila_mutex);
    pthread_mutex_unlock(&estat_mutex);
    return NULL;
}