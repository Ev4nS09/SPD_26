/*******************************************
Life 1.0
Copyright 2002, David Joiner and
  The Shodor Education Foundation, Inc.
Updated 2010, Andrew Fitz Gibbon and
  The Shodor Education Foundation, Inc.
Updated Summer 2010, Tiago Sommer Damasceno and
  The Shodor Education Foundation, Inc.
  
A C implementation of Conway's Game of Life.

To run:
./Life [Rows] [Columns] [Generations] [Display]

See the README included in this directory for
more detailed information.
*******************************************/

#include "Life.h"		// For function's definitions and instructions.
#include "Defaults.h" 	// For Life's constants
#include <bits/time.h>

int main(int argc, char ** argv) {

	int count;			//	Variable used to perform the for-loop.

	struct life_t life; //	Variable that contains: 	size, throttle, 
						//ncols, nrows, generations, do_display, infile, outfile.     

    struct timespec start, stop;

	/*
		Initializes all life's variables
	*/
	init(&life, &argc, &argv);

	/*
		Loop containing instructions to simulate 
		the evolution of one generation.
	*/

    double time = 0;

	for (count = 0; count < life.generations; count++) {

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		copy_bounds(&life);	//	Copy neighbor's states(DEAD or ALIVE) into a two dimensional
							//array which will evaluated by the Life's rules

		eval_rules(&life);	//	Finds out how many neighbor's each cell has then updates 
							//the cell's state to DEAD or ALIVE.

		update_grid(&life);	//	Updates the old grid with the state value of each cell.
        
    clock_gettime(CLOCK_MONOTONIC_RAW, &stop);

    time += ((double) ((long) (stop.tv_nsec - start.tv_nsec))) / 1000000;
	}


	cleanup(&life);			// Writes the coordinates of all the ALIVE cells into the output files
							//and frees all the memory used by the program
                            //
    
    printf("%8f\n", time / life.generations);
;


	exit(EXIT_SUCCESS);
}
