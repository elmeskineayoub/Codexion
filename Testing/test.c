#include <stdio.h>
#include <unistd.h>

int main()
{
    fork();    
    fork();
    fork();
    printf("My name is: Ayoub Elmeskine - id:%d\n", getpid());
}
