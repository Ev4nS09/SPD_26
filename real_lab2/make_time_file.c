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


    //File related variables
    FILE* read_file = fopen(argv[1], "r");
    FILE* write_file= fopen(argv[2], "w");
    int trials = atoi(argv[3]);

    //Calculation related variables
    int counter = 0;
    double average = 0;
    double internal_average = 0;
    double sd = 0; //standard deviation
                   
    //Array variables
    double times[trials - 1];

    //input variables
    int minutes;
    double seconds;
    double internal_time;


    //Calculates average time and internal time
    while(fscanf(read_file, "%lf\nreal %d m %lf s", &internal_time, &minutes, &seconds) != EOF)
    {
        double time = (double) (minutes * 60) + seconds; 

        if(counter > 0)
        {
            times[counter - 1] = time;

            fprintf(write_file, "%d: %lf s\n", counter, time);

            average += time;
            internal_average += internal_time;
        }

        counter++;
    }


    average = average / ((double) counter - 1);
    internal_average = internal_average / ((double) counter - 1);

    //Calculates standard deviation
    for(int i = 0; i < trials - 1; i++)
    {
        sd += pow(times[i] - average, 2);
    }
    

    sd = sqrt(sd / (double) trials);

    //printf("%lf\n", internal_average);
    fprintf(write_file, "Average internal time: %lf\n", internal_average);
    fprintf(write_file, "Average time: %lf\n", average);
    fprintf(write_file, "Standard deviation: %lf\n", sd);

    printf("percentage: %lf\n", (sd / average) * 100);


    fclose(write_file);
    fclose(read_file);


    if((sd / average) * 100 > 4) //Checks if the deviation was too high
       exit(1); 


    return 0;
}
