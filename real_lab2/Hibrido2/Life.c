#include "Life.h"		// For function's definitions and instructions.
#include "Defaults.h" 	// For Life's constants

int main(int argc, char ** argv) {
	int count;
	struct life_t life;

	struct timespec t_init_start, t_init_end;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t_init_start);
	init(&life, &argc, &argv);
	clock_gettime(CLOCK_MONOTONIC_RAW, &t_init_end);
	double t_init = (t_init_end.tv_sec - t_init_start.tv_sec) +
	                (t_init_end.tv_nsec - t_init_start.tv_nsec) / 1e9;
	printf("Rank %d: init took %.6fs\n", life.rank, t_init);

    double t_comm_start = 0;
    double t_comm_end = 0;
    
    /*
	for (count = 0; count < life.generations; count++) {
        
        t_comm_start = MPI_Wtime();
        exchange_halos(&life);
        t_comm_end = MPI_Wtime();
        printf("Rank %d, gen: %d exchange_halos=%.6fs\n", life.rank, count, t_comm_end - t_comm_start);

        t_comm_start = MPI_Wtime();
		copy_bounds(&life);
        t_comm_end = MPI_Wtime();
        printf("Rank %d, gen: %d copy_bounds=%.6fs\n", life.rank, count, t_comm_end - t_comm_start);

        t_comm_start = MPI_Wtime();
		eval_rules(&life);
        t_comm_end = MPI_Wtime();
        printf("Rank %d, gen: %d eval_rules=%.6fs\n", life.rank, count, t_comm_end - t_comm_start);

        t_comm_start = MPI_Wtime();
		update_grid(&life);
        t_comm_end = MPI_Wtime();
        printf("Rank %d, gen: %d update_grid=%.6fs\n", life.rank, count, t_comm_end - t_comm_start);
	}
    */

	for (count = 0; count < life.generations; count++) {
        
        exchange_halos(&life);

		copy_bounds(&life);

		eval_rules(&life);

		update_grid(&life);
	}

	cleanup(&life);
	exit(EXIT_SUCCESS);
}
