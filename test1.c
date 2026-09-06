#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 4
#define TOTAL_OPS 40000000

long global_counter = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *contended_worker(void *arg)
{
    long ops_per_thread = TOTAL_OPS / NUM_THREADS;

    for (long i = 0; i < ops_per_thread; i++) {
        // CRITICAL MISTAKE: Locking inside the hot loop
        pthread_mutex_lock(&lock);
        global_counter++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, contended_worker, NULL);

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    printf("Final Counter: %ld\n", global_counter);
    return 0;
}