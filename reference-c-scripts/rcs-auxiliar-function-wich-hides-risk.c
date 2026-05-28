#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Mutex for serializing log access
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
// Mutex for protecting bank updates
pthread_mutex_t banco_mutex = PTHREAD_MUTEX_INITIALIZER;

// Register a log entry by locking the log mutex
void registrar_log()
{
    pthread_mutex_lock(&log_mutex);
    usleep(1000); // simulate work while holding the log lock
    pthread_mutex_unlock(&log_mutex);
}

// Update bank data while holding the bank mutex
void atualizar_banco()
{
    pthread_mutex_lock(&banco_mutex);
    usleep(1000);
    registrar_log();
    pthread_mutex_unlock(&banco_mutex);
}

// Thread enters by locking log_mutex first, then calls a function that locks banco_mutex
// and then attempts to lock log_mutex again. This creates a hidden lock ordering problem.
void *thread1(void *arg)
{
    pthread_mutex_lock(&log_mutex);

    usleep(1000);
    atualizar_banco();
    pthread_mutex_unlock(&log_mutex);
    return NULL;
}