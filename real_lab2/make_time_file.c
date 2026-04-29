#include <stdio.h>

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        printf("Too few arguments\n");
        return 0;
    }

    FILE* read_file = fopen(argv[1], "r");
    FILE* write_file= fopen(argv[2], "w");

    int counter = 0;
    double average = 0;

    int minutes;
    double seconds;

    while(fscanf(read_file, " real %d m %lf s", &minutes, &seconds) != EOF)
    {
        double time = (double) (minutes * 60) + seconds; 
        fprintf(write_file, "%d: %lf s\n", counter, time);

        average += time;
        counter++;
    }

    fprintf(write_file, "Average time: %lf\n", average / (double) counter);

    fclose(read_file);
    fclose(write_file);


    return 0;
}
