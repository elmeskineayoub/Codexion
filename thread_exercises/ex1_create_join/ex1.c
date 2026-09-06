#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 5

// PREDICT: before running, write here what you think will print.
// Will the numbers appear in order 0,1,2,3,4? Why or why not?

void *thread_func(void *arg)
{
	// TODO: cast arg back to an int (or int*, your choice — think about which
	// is safer and why) and print "thread %d started\n" then "thread %d finished\n"
	int x;
	x = (int)arg;
	printf("thread %d started\n", x);
	printf("thread %d finished\n", x);
	return (NULL);
}

int main(void)
{
	pthread_t threads[NUM_THREADS];

	// TODO: spawn NUM_THREADS threads with pthread_create, passing each
	// thread its index.
	//
	// FIRST attempt: pass &i directly from the loop variable `i`.
	// Run it 5 times. Look closely at the printed numbers. Are they always
	// 0,1,2,3,4? Are they ever the same number twice? Are they ever wrong?
	//
	// Once you've SEEN the bug, fix it (hint: each thread needs its own
	// piece of memory holding its index — the loop variable `i` is shared
	// and changes while threads are still starting).
	

	// TODO: join all threads before returning.

	return (0);
}
