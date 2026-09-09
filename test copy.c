#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREAD_NUMBER 2

int numb[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

void *work(void *arg)
{
	int index = *(int*)arg;
	int sum = 0; 
	while(index < 5)
	{
		sum += numb[index];
		index++;
	}
	printf("the local sum = %d\n", sum);
	*(int*) arg = sum;
	return arg;
}

int main() 
{
    pthread_t thread[THREAD_NUMBER];
	int i;
	for(i = 0; i < THREAD_NUMBER; i++)
	{
		int *x = malloc(sizeof(int));
		*x = i;
		pthread_create(&thread[i], NULL, &work, x);
	}
	int big_some = 0;
    for (i = 0; i < THREAD_NUMBER; i++)
    {
		int *r;
		pthread_join(thread[i], (void **)&r);
		big_some += *r;
		free(r);
	}
	printf("the sum of all is %d", big_some);
    return 0;
}
