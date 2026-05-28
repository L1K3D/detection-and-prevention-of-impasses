#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Initialization of global synchronization primitives
pthread_mutex_t impressora = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t spooler_mutex = PTHREAD_MUTEX_INITIALIZER;

// Simulated shared resource representing the spooler queue
int trabalhos_no_spooler = 0;

void *usuario1(void *arg)
{
    // FIX: Users no longer interact with the printer hardware directly.
    // They only acquire the spooler lock to submit their jobs safely.
    pthread_mutex_lock(&spooler_mutex);
    usleep(500); // Simulating time taken to enqueue a document
    trabalhos_no_spooler++;
    printf("Usuario 1: Documento submetido com exito ao spooler.\n");
    pthread_mutex_unlock(&spooler_mutex);

    return NULL;
}

void *usuario2(void *arg)
{
    // Identical safe architecture for the second user thread.
    pthread_mutex_lock(&spooler_mutex);
    usleep(500);
    trabalhos_no_spooler++;
    printf("Usuario 2: Documento submetido com exito ao spooler.\n");
    pthread_mutex_unlock(&spooler_mutex);

    return NULL;
}

// Independent background process representing the Spooler Daemon
void *daemon_spooler(void *arg)
{
    int trabalhos_pendentes = 0;

    // The daemon safely checks the spooler queue
    pthread_mutex_lock(&spooler_mutex);
    if (trabalhos_no_spooler > 0)
    {
        trabalhos_pendentes = trabalhos_no_spooler;
        trabalhos_no_spooler = 0; // Clears the queue after fetching jobs
    }
    pthread_mutex_unlock(&spooler_mutex);

    // If there are jobs, the daemon is the ONLY entity that directly locks the printer
    if (trabalhos_pendentes > 0)
    {
        pthread_mutex_lock(&impressora);
        printf("Daemon do Spooler: Imprimindo %d trabalho(s) pendente(s)...\n", trabalhos_pendentes);
        usleep(1500); // Simulating hardware printing time
        printf("Daemon do Spooler: Impressao concluida de forma segura.\n");
        pthread_mutex_unlock(&impressora);
    }
    else
    {
        printf("Daemon do Spooler: Nenhum trabalho pendente na fila.\n");
    }

    return NULL;
}

int main()
{
    pthread_t t_usr1, t_usr2, t_daemon;

    // Dispatching user threads concurrently to simulate simultaneous job submission
    pthread_create(&t_usr1, NULL, usuario1, NULL);
    pthread_create(&t_usr2, NULL, usuario2, NULL);

    // Synchronizing user completion before invoking the daemon for demonstration purposes
    pthread_join(t_usr1, NULL);
    pthread_join(t_usr2, NULL);

    // Launching the daemon to process the accumulated spooler queue
    pthread_create(&t_daemon, NULL, daemon_spooler, NULL);
    pthread_join(t_daemon, NULL);

    printf("Operacoes de spooling e impressao finalizadas sem intertravamentos.\n");

    // Resource cleanup
    pthread_mutex_destroy(&impressora);
    pthread_mutex_destroy(&spooler_mutex);

    return 0;
}