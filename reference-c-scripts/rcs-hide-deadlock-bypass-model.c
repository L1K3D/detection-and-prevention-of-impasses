#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Mutex protecting cache updates
pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;
// Mutex protecting disk writes
pthread_mutex_t disco_mutex = PTHREAD_MUTEX_INITIALIZER;

// Write to disk while holding the disk mutex
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

// Flush disk data into cache by locking disco_mutex first, then cache_mutex
// This inconsistent lock ordering creates a deadlock risk with atualizar_cache().
void flush_disco_para_cache()
{
    pthread_mutex_lock(&disco_mutex);
    usleep(1000);
    pthread_mutex_lock(&cache_mutex);

    pthread_mutex_unlock(&cache_mutex);
    pthread_mutex_unlock(&disco_mutex);
}