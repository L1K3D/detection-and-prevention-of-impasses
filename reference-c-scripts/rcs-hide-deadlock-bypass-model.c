#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t disco_mutex = PTHREAD_MUTEX_INITIALIZER;

void gravar_disco()
{
    pthread_mutex_lock(&disco_mutex);
    usleep(1000);
    pthread_mutex_unlock(&disco_mutex);
}

void atualizar_cache()
{
    pthread_mutex_lock(&cache_mutex);
    usleep(1000);
    gravar_disco();
    pthread_mutex_unlock(&cache_mutex);
}

void flush_disco_para_cache()
{
    pthread_mutex_lock(&disco_mutex);
    usleep(1000);
    pthread_mutex_lock(&cache_mutex);

    pthread_mutex_unlock(&cache_mutex);
    pthread_mutex_unlock(&disco_mutex);
}