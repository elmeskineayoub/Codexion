#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 4
#define INCREMENTS 1000000

int counter = 0;
pthread_mutex_t counter_lock;

void *increment_func(void *arg)
{
	(void)arg;
	// TODO (part A): loop INCREMENTS times. Lock counter_lock, counter++,
	// unlock counter_lock. Every single increment gets its own lock/unlock
	// pair (yes, this is slow — that's expected, correctness first).
	for (int i = 0; i < INCREMENTS; i++)
	{	
		pthread_mutex_lock(&counter_lock);
		if (i == 500)
			return NULL;
		counter++;
		pthread_mutex_unlock(&counter_lock);
	}
	return (NULL);
}

int main(void)
{
	pthread_t threads[NUM_THREADS];
	pthread_mutex_init(&counter_lock, NULL);
	for (int i = 0; i < NUM_THREADS; i++)
		pthread_create(&threads[i], NULL, &increment_func, &counter_lock);
	for (int i = 0; i < NUM_THREADS; i++)
		pthread_join(threads[i], NULL);	
	// TODO: pthread_mutex_init(&counter_lock, NULL);
	// TODO: spawn NUM_THREADS threads, join them all.

	printf("expected: %d\n", NUM_THREADS * INCREMENTS);
	printf("actual:   %d\n", counter);

	// Run 5 times. It should ALWAYS match now. If it doesn't, you have a
	// bug in your locking — find it before moving on.
	pthread_mutex_destroy(&counter_lock);
	// TODO: pthread_mutex_destroy(&counter_lock);
	return (0);
}

/*
 * PART B — do this only after part A works correctly 5/5 runs.
 *
 * Make a COPY of this file as ex3_broken.c. In that copy, introduce a bug:
 * add an early `return (NULL);` inside the loop under some condition
 * (e.g. `if (i == 500) return (NULL);`) placed AFTER the lock but BEFORE
 * the unlock. Run it.
 *
 * What happens? Does the program finish? Does it hang? Use Ctrl+C and/or
 * `gdb` (`info threads`, `thread apply all bt`) to look at what the other
 * threads are stuck doing. This is what a real deadlock/hang looks like
 * from the outside — recognizing this exact symptom is a skill you need
 * for Codexion.
 */
