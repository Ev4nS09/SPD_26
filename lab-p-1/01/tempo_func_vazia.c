#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

void empty(){}

int main(int argc, char* argv[])
{
    if(argc >= 2 && ( argv[1][0] < '0' || argv[1][0] > '9'))
    {
        fprintf(stderr, "Usage: %s <positive_integer>\n", argv[0]);
        exit(1);
    }

    

    int iterations = argc >= 2 ? atoll(argv[1]) : 1;
    long double elapsed = 0;
    struct timespec start, stop;

    for(int i = 0; i < iterations; i++)
    {	
        clock_gettime(CLOCK_REALTIME, &start);

        empty();

        clock_gettime(CLOCK_REALTIME, &stop);

    }

    elapsed += (double)(stop.tv_nsec - start.tv_nsec);

    printf("Elapsed time: Loop time = %8f\n",(double) elapsed / iterations);

    exit(0);
}
