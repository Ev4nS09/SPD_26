
// substituir gettimeofday pq POSIX.1-2008 marca gettimeofday() como obsoleto

// ver https://manpages.debian.org/trixie/manpages-dev/clock_gettime.2.en.html
// notar a chamada clock_getres() permite obter a precisão de um relógio
// ver no final da página do manual o exemplo de utilização e o código fonte desse exemplo.


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
    int pid = 0;

    int x = 1;

    for(int i = 0; i < iterations; i++)
    {	
        clock_gettime(CLOCK_REALTIME, &start);

        //pid = getpid();

        empty();

        //printf("Hello World\n");

        
        /*for(long j = 0; j < 2000000; j++)
        {
           int t = j % 2; 
        }*/
        
        clock_gettime(CLOCK_REALTIME, &stop);

        //elapsed equals miliseconds
        //elapsed += (double)(stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / 1000000.0;


        //elapsed equals nano seconds
        elapsed += (double)(stop.tv_nsec - start.tv_nsec);
    }

    printf("Elapsed time: Loop time = %8f\n",(double) elapsed / iterations);

    exit(0);
}
