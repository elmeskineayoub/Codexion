#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

int ready = 0;
pthread_mutex_t lock;
pthread_cond_t cond;

// PREDICT: nobody will ever signal `cond` in this exercise. After ~300ms,
// what should happen to the consumer thread? What does pthread_cond_timedwait
// return in that case?
void build_deadline(struct timespec *ts, int ms)
{
    clock_gettime(CLOCK_REALTIME, ts);
    ts->tv_sec  += ms / 1000;
    ts->tv_nsec += (long)(ms % 1000) * 1000000L;

    if (ts->tv_nsec >= 1000000000) 
	{
        ts->tv_sec  += 1;
        ts->tv_nsec -= 1000000000;
    }
}
// TODO: write a helper that fills a struct timespec with "now + ms milliseconds"
// void build_deadline(struct timespec *ts, int ms)
// {
//     clock_gettime(CLOCK_REALTIME, ts);
//     ts->tv_sec  += ms / 1000;
//     ts->tv_nsec += (ms % 1000) * 1000000;
//     // TODO: handle overflow — tv_nsec must stay below 1,000,000,000.
//     // If it overflows, what do you add to tv_sec and what do you subtract
//     // from tv_nsec? Work this out on paper before coding it.
// }

void *consumer(void *arg)
{
	(void)arg;
	struct timespec deadline;
	// TODO: build_deadline(&deadline, 300);
	build_deadline(&deadline, 300);
	pthread_mutex_lock(&lock);
	int rc = 0;

	while(!ready && rc == 0)
		rc = pthread_cond_timedwait(&cond, &lock, &deadline);

	if(rc == ETIMEDOUT)
		printf("timed out, nobody signaled");
	else
		printf("got it");

		pthread_mutex_unlock(&lock);
	// TODO:
	// lock
	// int rc = 0;
	// while (!ready && rc == 0)
	//     rc = pthread_cond_timedwait(&cond, &lock, &deadline);
	// if (rc == ETIMEDOUT) print "timed out, nobody signaled"
	// else print "got it"
	// unlock
	return (NULL);
}

int main(void)
{
	pthread_t c;

	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&cond, NULL);

	pthread_create(&c, NULL, &consumer, NULL);

	pthread_join(c, NULL);
	// TODO: init mutex + cond
	// TODO: spawn consumer, join it
	// (no producer at all in this exercise — that's the point)
	// TODO: destroy mutex + cond
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cond);
	return (0);
}
