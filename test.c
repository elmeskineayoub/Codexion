#include <unistd.h>
#include <stdio.h>
#include <pthread.h>


void *routine()
{
	printf("Test from threads\n");
	return NULL;
}

int main()
{
	pthread_t T1;
	pthread_create(&T1, NULL, routine, NULL);
	pthread_join(T1,NULL);
	return 0;
}
