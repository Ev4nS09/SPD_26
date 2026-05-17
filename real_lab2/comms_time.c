#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int N = 1000000; // number of integers to send
    int *buffer = (int*) malloc(N * sizeof(int));

    // Initialize buffer
    for (int i = 0; i < N; i++) buffer[i] = rank;

    MPI_Barrier(MPI_COMM_WORLD); // synchronize before timing

    if (rank == 0) {
        // Rank 0 sends to rank 1
        MPI_Send(buffer, N, MPI_INT, 1, 0, MPI_COMM_WORLD);
        // Rank 0 receives reply
        MPI_Recv(buffer, N, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else if (rank == 1) {
        // Rank 1 receives from rank 0
        MPI_Recv(buffer, N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Rank 1 sends reply
        MPI_Send(buffer, N, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    free(buffer);
    MPI_Finalize();
    return 0;
}
