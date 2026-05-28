#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Global synchronization primitives defining the shared resources
pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t disco_mutex = PTHREAD_MUTEX_INITIALIZER;

void gravar_disco()
{
    // The disk mutex is acquired safely based on the caller's established hierarchy
    pthread_mutex_lock(&disco_mutex);
    usleep(1000);
    printf("Operacao de I/O concluida: Dado gravado no disco.\n");
    pthread_mutex_unlock(&disco_mutex);
}

void atualizar_cache()
{
    // Acquires the cache mutex first.
    // The effective global lock order here implicitly becomes: Cache -> Disk.
    pthread_mutex_lock(&cache_mutex);
    usleep(1000);
    printf("Memoria transitoria: Cache atualizado.\n");

    // Calls the external function which will subsequently lock the disk mutex
    gravar_disco();

    pthread_mutex_unlock(&cache_mutex);
}

void flush_disco_para_cache()
{
    // FIX: Architectural compliance with the global locking hierarchy.
    // To eradicate the hidden deadlock, this function MUST acquire the cache mutex
    // BEFORE the disk mutex, mirroring the exact order established by atualizar_cache().
    pthread_mutex_lock(&cache_mutex);
    pthread_mutex_lock(&disco_mutex);

    usleep(1000);
    printf("Procedimento de flush concluido com integridade estrutural.\n");

    // Relinquishing system resources symmetrically in the reverse order of acquisition
    pthread_mutex_unlock(&disco_mutex);
    pthread_mutex_unlock(&cache_mutex);
}

// Auxiliary thread functions to demonstrate concurrent execution
void *thread_cache(void *arg)
{
    atualizar_cache();
    return NULL;
}

void *thread_flush(void *arg)
{
    flush_disco_para_cache();
    return NULL;
}

int main()
{
    pthread_t t1, t2;

    // Instantiating the execution threads concurrently
    if (pthread_create(&t1, NULL, thread_cache, NULL) != 0)
    {
        perror("Falha ao instanciar thread_cache");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&t2, NULL, thread_flush, NULL) != 0)
    {
        perror("Falha ao instanciar thread_flush");
        exit(EXIT_FAILURE);
    }

    // Synchronizing the main process execution boundary
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Todas as transacoes de modulos foram concluídas estritamente sem intertravamentos.\n");

    // Graceful destruction of the synchronization objects
    pthread_mutex_destroy(&cache_mutex);
    pthread_mutex_destroy(&disco_mutex);

    return 0;
}