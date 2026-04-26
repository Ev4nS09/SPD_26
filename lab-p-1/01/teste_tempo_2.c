
// substituir gettimeofday pq POSIX.1-2008 marca gettimeofday() como obsoleto

// ver https://manpages.debian.org/trixie/manpages-dev/clock_gettime.2.en.html
// notar a chamada clock_getres() permite obter a precisão de um relógio
// ver no final da página do manual o exemplo de utilização e o código fonte desse exemplo.


#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

double rtc()
{
        struct timeval time;
        gettimeofday(&time,NULL);
        return ( (double)(time.tv_sec*1000000+time.tv_usec)/1000000 );
}

int main()
{
        double elapsed, rtc();
        int t;
	
        elapsed=rtc();
		/* Operation to be timed here */
                for(long i=0; i<2000000; i++)
                        t = i mod(2);
		/* Operation to be timed here */

        elapsed=rtc()-elapsed;

        printf( "   Elapsed time: Loop time = %8f  \n",	elapsed );

        exit (0);
}



