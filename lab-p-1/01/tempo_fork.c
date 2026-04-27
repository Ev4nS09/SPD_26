#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char* argv[])
{
    int status;
    int pid;
    struct timespec start, stop;

    clock_gettime(CLOCK_REALTIME, &start);

    pid = fork();
    wait(&status);

    clock_gettime(CLOCK_REALTIME, &stop);
    
    if(pid == 0)
        exit(0);

    double elapsed = (double)(stop.tv_nsec - start.tv_nsec);

    printf("Elapsed time: Loop time from pid %d = %8f\n",pid, (double) elapsed );
    

    exit(0);
}
