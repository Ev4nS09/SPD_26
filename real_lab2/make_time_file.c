#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char* argv[])
{
    if(argc < 4)
    {
        printf("Too few arguments\n");
        return 0;
    }


    FILE* read_file = fopen(argv[1], "r");
    FILE* write_file= fopen(argv[2], "w");
    int trials = atoi(argv[3]);

    int counter = 0;
    double average = 0;
    double sd = 0; //standard deviation
    double times[trials];

    int minutes;
    double seconds;


    //Calculates average time
    while(fscanf(read_file, " real %d m %lf s", &minutes, &seconds) != EOF)
    {
        double time = (double) (minutes * 60) + seconds; 
        times[counter] = time;
        fprintf(write_file, "%d: %lf s\n", counter, time);

        average += time;
        counter++;
    }


    average = average / (double) counter;

    //Calculates standard deviation
    for(int i = 0; i < trials; i++)
    {
        sd += pow(times[0] - average, 2);
    }
    

    sd = sqrt(sd / (double) trials);

    fprintf(write_file, "Average time: %lf\n", average);
    fprintf(write_file, "Standard deviation: %lf\n", sd);

    printf("percentage: %lf\n", (sd / average) * 100);


    fclose(read_file);
    fclose(write_file);


    if((sd / average) * 100 > 9) //Checks if the deviation was too high
       exit(1); 


    return 0;
}
