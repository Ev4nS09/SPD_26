/***********************************************
 * FILE: mpi_hello.c
 * DESCRIPTION:
 * MPI tutorial example code: Simple hello world program
 * AUTHOR: Blaise Barney
 * LAST REVISED: 03/05/10
 ***********************************************/

#include "mpi.h"
#include <stdio.h>
#include <unistd.h>

#define MASTER 0

void print_func()
{
    printf("Im a function\n");
}

void init(int argc, char *argv[])
{
    int numtasks, taskid, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Get_processor_name(hostname, &len);

    printf("Hello from task %d on %s!\n", taskid, hostname);

    if (taskid == MASTER)
        printf("MASTER: Number of MPI tasks is: %d\n", numtasks);

    MPI_Finalize();
}

int main(int argc, char *argv[])
{

    printf("Im suposed to be just 1\n");

    init(argc, argv);
    print_func();

    return 0;
}
