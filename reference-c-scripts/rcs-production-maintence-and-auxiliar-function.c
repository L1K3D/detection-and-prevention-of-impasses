#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t fila_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t estat_mutex = PTHREAD_MUTEX_INITIALIZER;

void atualizar_estatisticas()
{
    pthread_mutex_lock(&estat_mutex);
    usleep(1000);
    pthread_mutex_unlock(&estat_mutex);
}

void *produtor(void *arg)
{
    pthread_mutex_lock(&fila_mutex);
    usleep(1000);
    atualizar_estatisticas();
    pthread_mutex_unlock(&fila_mutex);
    return NULL;
}

void *manutencao(void *arg)
{
    pthread_mutex_lock(&estat_mutex);
    usleep(1000);
    pthread_mutex_lock(&fila_mutex);

    pthread_mutex_unlock(&fila_mutex);
    pthread_mutex_unlock(&estat_mutex);
    return NULL;
}