#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_CONSUMERS 3

int ready = 0;
pthread_mutex_t lock;
pthread_cond_t cond;

// PREDICT: with pthread_cond_signal (not broadcast), how many of the 3
// consumers do you expect to wake up and print? Write your guess first.

void *consumer(void *arg)
{
	long id = (long)arg;
	// TODO:
	// lock
	// while (!ready) pthread_cond_wait(&cond, &lock)
	// print "consumer %ld got it\n", id
	// unlock
	return (NULL);
}

void *producer(void *arg)
{
	(void)arg;
	usleep(300000);
	// TODO:
	// lock, set ready = 1, THEN CHOOSE ONE:
	//   pthread_cond_signal(&cond);      <- try this first
	// unlock
	return (NULL);
}

int main(void)
{
	pthread_t consumers[NUM_CONSUMERS];
	pthread_t p;

	// TODO: init mutex + cond
	// TODO: spawn NUM_CONSUMERS consumer threads, then the producer thread
	// TODO: join everything

	// After running once with cond_signal, change producer's call to
	// pthread_cond_broadcast(&cond) and run again. Compare how many
	// consumers printed each time.

	return (0);
}
