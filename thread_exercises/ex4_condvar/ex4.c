#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int ready = 0;
pthread_mutex_t lock;
pthread_cond_t cond;

// PREDICT: what order will the printfs appear in? Will "got it" ever print
// before "producer: setting ready"?

void *producer(void *arg)
{
	(void)arg;
	usleep(500000); // 500ms
	// TODO:
	// lock the mutex
	// set ready = 1
	// print "producer: setting ready"
	// signal the cond var
	// unlock the mutex
	return (NULL);
}

void *consumer(void *arg)
{
	(void)arg;
	// TODO:
	// lock the mutex
	// while (!ready) pthread_cond_wait(&cond, &lock);   <-- use WHILE, not IF
	//   (try `if` first on purpose, see hints)
	// print "consumer: got it"
	// unlock the mutex
	return (NULL);
}

int main(void)
{
	pthread_t p, c;

	// TODO: init mutex and cond var
	// TODO: create producer and consumer threads
	// TODO: join both
	// TODO: destroy mutex and cond var

	return (0);
}
