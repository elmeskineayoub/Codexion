#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREAD_NUMBER 3

int primes[10] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};

int len(char *list)
{
	int i, counter;
	while(list[i])
		counter++;
	return counter;
}

void* calculate_half_sum(void *arg)
{
    int x = *(int *)arg;
    free(arg);
	int half = len(primes) / 2;
	int sum;
	for (int i = 0; i <= half; i++)
		sum += primes[i];
    printf("%d ", sum);
    return NULL;
}

void *calculate_the_sum(void *arg)
{

}

int main() 
{
    pthread_t thread[THREAD_NUMBER];
	int *x = malloc(sizeof(int));
	pthread_create(&thread[0], NULL, &calculate_half_sum, x);
	pthread_create(&thread[1], NULL, &calculate_half_sum, x);
	pthread_create(&thread[2], NULL, &calculate_the_sum, x);



    for (int j = 0; j < THREAD_NUMBER; j++)
        pthread_join(thread[j], NULL);

    printf("\n");
    return 0;
}
