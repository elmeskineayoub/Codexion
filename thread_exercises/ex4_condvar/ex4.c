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
	pthread_mutex_lock(&lock);
	ready = 1;
	printf("producer: setting ready\n");
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);
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
	pthread_mutex_lock(&lock);
	if(!ready)
		pthread_cond_wait(&cond, &lock);
	printf("consumer: got it\n");
	pthread_mutex_unlock(&lock);
	return (NULL);
}

int main(void)
{
	pthread_t p, c;

	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&cond, NULL);

	pthread_create(&p, NULL, &producer, NULL);
	pthread_create(&c, NULL, &consumer, NULL);

	pthread_join(p, NULL);
	pthread_join(c, NULL);

	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cond);
	// TODO: init mutex and cond var
	// TODO: create producer and consumer threads
	// TODO: join both
	// TODO: destroy mutex and cond var

	return (0);
}
