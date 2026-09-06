#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 4
#define INCREMENTS 1000000

// Deliberately global for this exercise only — Codexion forbids globals,
// this is purely to demonstrate the race.
int counter = 0;

// PREDICT: what should the final value of counter be after all threads
// finish? Write your prediction here BEFORE running anything.

void *increment_func(void *arg)
{
	(void)arg;
	// TODO: loop INCREMENTS times, doing counter++ each time.
	// No mutex. On purpose.
	return (NULL);
}

int main(void)
{
	pthread_t threads[NUM_THREADS];

	// TODO: spawn NUM_THREADS threads all running increment_func.
	// TODO: join them all.

	printf("expected: %d\n", NUM_THREADS * INCREMENTS);
	printf("actual:   %d\n", counter);

	// Run this binary 5 times in a row (./ex2 ; ./ex2 ; ./ex2 ; ./ex2 ; ./ex2)
	// Is "actual" ever equal to "expected"? Is it different each time?

	return (0);
}
