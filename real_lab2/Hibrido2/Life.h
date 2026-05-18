/*******************************************
Life 1.0
Copyright 2002, David Joiner and
  The Shodor Education Foundation, Inc.
Updated 2010, Andrew Fitz Gibbon and
  The Shodor Education Foundation, Inc.
Updated Summer 2010, Tiago Sommer Damasceno and
  The Shodor Education Foundation, Inc.
Updated 2024, MPI Domain Decomposition (row-based)
*******************************************/

#ifndef BCCD_LIFE_H
#define BCCD_LIFE_H

#include "XLife.h"    // For display routines
#include "Defaults.h" // For Life's constants
#include <mpi.h>

#include <time.h>     // For seeding random
#include <stdlib.h>   // For malloc et al.
#include <stdbool.h>  // For true/false
#include <getopt.h>   // For argument processing
#include <stdio.h>    // For file i/o
#include <string.h>
#include <unistd.h>
#include <omp.h>      // For parallelization (OpenMP)

/* ============================================================
   MPI DOMAIN DECOMPOSITION — HOW IT WORKS
   ============================================================
   The global grid has dimensions  global_nrows x ncols.
   We split along the ROW axis so that each MPI rank owns a
   horizontal slab:

       Rank 0  rows  1 .. local_nrows          (+ ghost rows 0, local_nrows+1)
       Rank 1  rows  1 .. local_nrows          (+ ghost rows 0, local_nrows+1)
       ...

   Example — 16x16 grid, 2 processes:
       Rank 0: rows 1-8   of the global grid
       Rank 1: rows 9-16  of the global grid

   After every generation each rank sends its real top row to
   its upper neighbour's bottom ghost row, and its real bottom
   row to its lower neighbour's top ghost row.  This is called
   a "halo exchange".  MPI_Sendrecv is used so that both
   directions happen simultaneously without deadlock.

   Periodic (wrap-around) boundaries are maintained:
     - Top/bottom wrapping is done via the halo exchange
       between rank 0 and rank (size-1).
     - Left/right wrapping is done locally inside copy_bounds().

   life->nrows      = LOCAL row count for this process
   life->global_nrows = total rows in the full grid
   ============================================================ */

/* ---- forward declarations ---- */
int               init (struct life_t * life, int * c, char *** v);
void        eval_rules (struct life_t * life);
void       copy_bounds (struct life_t * life);
void    exchange_halos (struct life_t * life);          /* NEW */
void       update_grid (struct life_t * life);
void          throttle (struct life_t * life);
void    allocate_grids (struct life_t * life);
void        init_grids (struct life_t * life);
void        write_grid (struct life_t * life);
void        free_grids (struct life_t * life);
double     rand_double ();
void    randomize_grid (struct life_t * life, double prob);
void           cleanup (struct life_t * life);
void        parse_args (struct life_t * life, int argc, char ** argv);
void             usage ();

/* ==============================================================
   init()
     Initialises MPI, divides the global row count among ranks
     WEIGHTED by each rank's OpenMP thread count, and sets up
     each process's local grid.

     Example — 16 rows, rank 0 has 4 threads, rank 1 has 2 threads:
       total_threads = 6
       rank 0 weight = 4/6 → gets ~11 rows
       rank 1 weight = 2/6 → gets ~5  rows

     This means the machine with more cores always gets more work,
     keeping both machines busy for roughly the same wall-clock time
     and minimising idle waiting at MPI_Sendrecv.
   ============================================================== */
int init (struct life_t * life, int * c, char *** v) {
    int argc     = *c;
    char ** argv = *v;

    /* ---- defaults ---- */
    life->rank         = 0;
    life->size         = 1;
    life->throttle     = -1;
    life->ncols        = DEFAULT_SIZE;
    life->nrows        = DEFAULT_SIZE;
    life->generations  = DEFAULT_GENS;
    life->do_display   = DEFAULT_DISP;
    life->infile       = NULL;
    life->outfile      = NULL;

    srandom(time(NULL));

    /* ---- MPI setup ---- */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &life->rank);
    MPI_Comm_size(MPI_COMM_WORLD, &life->size);

    parse_args(life, argc, argv);

    int global_nrows   = life->nrows;
    life->global_nrows = global_nrows;

    /* ---- Step 1: every rank reports its thread count to rank 0 ---- */
    int my_threads = omp_get_max_threads();
    int * all_threads = NULL;

    if (life->rank == 0)
        all_threads = (int *) malloc(sizeof(int) * life->size);

    /*
     * MPI_Gather collects one integer from every rank into
     * all_threads[] on rank 0, in rank order.
     */
    MPI_Gather(&my_threads, 1, MPI_INT,
               all_threads, 1, MPI_INT,
               0, MPI_COMM_WORLD);

    /* ---- Step 2: rank 0 computes weighted row counts ---- */
    int * row_counts  = (int *) malloc(sizeof(int) * life->size);
    int * row_offsets = (int *) malloc(sizeof(int) * life->size);

    if (life->rank == 0) {
        /*
         * Sum all thread counts to get the total "weight".
         * Each rank's share of rows = (its threads / total) * global_nrows.
         * We use integer rounding and fix any leftover rows by giving
         * them to rank 0.
         */
        int total_threads = 0;
        for (int r = 0; r < life->size; r++)
            total_threads += all_threads[r];

        printf("Weighted decomposition: total threads = %d\n", total_threads);

        int assigned = 0;
        for (int r = 0; r < life->size; r++) {
            if (r < life->size - 1) {
                /* Proportional share, rounded down */
                row_counts[r] = (all_threads[r] * global_nrows) / total_threads;
            } else {
                /* Last rank gets whatever rows are left — no rounding error */
                row_counts[r] = global_nrows - assigned;
            }
            assigned += row_counts[r];

            printf("  Rank %d: %d threads → %d rows\n",
                   r, all_threads[r], row_counts[r]);
        }

        /* Compute offsets (0-based, used to map local→global row index) */
        row_offsets[0] = 0;
        for (int r = 1; r < life->size; r++)
            row_offsets[r] = row_offsets[r-1] + row_counts[r-1];

        free(all_threads);
    }

    /* ---- Step 3: broadcast the counts and offsets to all ranks ---- */
    MPI_Bcast(row_counts,  life->size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(row_offsets, life->size, MPI_INT, 0, MPI_COMM_WORLD);

    /* Each rank picks its own slice */
    life->nrows      = row_counts[life->rank];
    life->row_offset = row_offsets[life->rank];

    free(row_counts);
    free(row_offsets);


    /* ---- Step 4: initialise the local grid ---- */
    init_grids(life);

    #pragma omp parallel for
for (int test = 0; test < 8; test++) {
    printf("Thread %d of %d running on rank %d\n",
           omp_get_thread_num(), omp_get_num_threads(), life->rank);
}
fflush(stdout);

    return 0;
}

/* ==============================================================
   exchange_halos()
     Exchanges ghost (halo) rows with the upper and lower
     neighbour processes so that each rank has up-to-date
     boundary data before calling eval_rules().

     Layout inside each rank's grid array  (row index):
       0            ← top ghost row    (filled from rank above)
       1 .. nrows   ← real rows owned by this rank
       nrows+1      ← bottom ghost row (filled from rank below)

     Periodic boundary: rank 0's top ghost comes from rank
     (size-1), and vice-versa.
   ============================================================== */
void exchange_halos (struct life_t * life) {
    int rank  = life->rank;
    int size  = life->size;
    int ncols = life->ncols;
    int nrows = life->nrows;      /* local row count */

    int ** grid = life->grid;

    /*
     * Neighbours (with wrap-around for periodic boundary).
     * The grid is split along rows, so "above" means the rank
     * with index (rank-1) and "below" means (rank+1).
     */
    int rank_above = (rank - 1 + size) % size;
    int rank_below = (rank + 1)        % size;

    MPI_Status status;

    /*
     * We need to send/receive a full row of (ncols+2) integers
     * (including the column ghost cells so corners are correct).
     *
     * Step 1: send our real TOP row (row 1) to the rank above;
     *         receive into our BOTTOM ghost row (row nrows+1)
     *         from the rank below.
     */
    MPI_Sendrecv(
        grid[1],          ncols + 2, MPI_INT, rank_above, 0,
        grid[nrows + 1],  ncols + 2, MPI_INT, rank_below, 0,
        MPI_COMM_WORLD, &status
    );

    /*
     * Step 2: send our real BOTTOM row (row nrows) to the rank
     *         below; receive into our TOP ghost row (row 0)
     *         from the rank above.
     */
    MPI_Sendrecv(
        grid[nrows], ncols + 2, MPI_INT, rank_below, 1,
        grid[0],     ncols + 2, MPI_INT, rank_above, 1,
        MPI_COMM_WORLD, &status
    );
}

/* ==============================================================
   eval_rules()
     Evaluate the rules of Life for each LOCAL cell.
     Ghost rows (row 0 and row nrows+1) must already be filled
     by exchange_halos() before this is called.
     OpenMP parallelises the outer loop across threads.
   ============================================================== */
void eval_rules (struct life_t * life) {
    int i, j, k, l, neighbors;

    int ncols = life->ncols;
    int nrows = life->nrows;          /* local */

    int ** grid      = life->grid;
    int ** next_grid = life->next_grid;

    #pragma omp parallel for private(neighbors,j,k,l)
    for (i = 1; i <= nrows; i++) {   /* iterate over LOCAL rows */
        for (j = 1; j <= ncols; j++) {
            neighbors = 0;

            /* Count the 8 neighbours. */
            for (k = i - 1; k <= i + 1; k++) {
                for (l = j - 1; l <= j + 1; l++) {
                    if (!(k == i && l == j) && grid[k][l] != DEAD)
                        neighbors++;
                }
            }

            /* Apply Conway's rules. */
            if (neighbors < LOWER_THRESH || neighbors > UPPER_THRESH)
                next_grid[i][j] = DEAD;
            else if (grid[i][j] != DEAD || neighbors == SPAWN_THRESH)
                next_grid[i][j] = grid[i][j] + 1;
        }
    }
}

/* ==============================================================
   copy_bounds()
     Handles the LEFT and RIGHT (column) periodic boundaries
     locally — these are never shared with other MPI ranks.
     Top and bottom boundaries are handled by exchange_halos().
   ============================================================== */
void copy_bounds (struct life_t * life) {
    int i, j;

    int size  = life->size;
    int ncols = life->ncols;
    int nrows = life->nrows;          /* local */

    int ** grid = life->grid;

    /*
     * Left/right column wrapping is always done locally,
     * regardless of how many MPI ranks there are.
     * Row 0 and row nrows+1 are ghost rows and also need their
     * column wrapping set — they are copied here after the halo
     * exchange so that corner cells are correct.
     */
    #pragma omp parallel for private(j)
    for (i = 0; i < nrows + 2; i++) {
        grid[i][0]        = grid[i][ncols];    // left ghost  = real right
        grid[i][ncols+1]  = grid[i][1];        // right ghost = real left
    }
    /*
     * NOTE: top (row 0) and bottom (row nrows+1) ghost rows are
     * NOT set here — they come from exchange_halos().
     * Only the column-direction corners need to be patched after
     * that exchange, which is done in the loop above since j=0
     * and j=nrows+1 are included.
     */
}

/* ==============================================================
   update_grid()
     Copies next_grid into grid for the next generation.
     Covers ghost rows too so they are ready for the halo exchange.
   ============================================================== */
void update_grid (struct life_t * life) {
    int i, j;
    int ncols = life->ncols;
    int nrows = life->nrows;          /* local */
    int ** grid      = life->grid;
    int ** next_grid = life->next_grid;

    #pragma omp parallel for private(j)
    for (i = 0; i < nrows + 2; i++)
        for (j = 0; j < ncols + 2; j++)
            grid[i][j] = next_grid[i][j];
}

/* ==============================================================
   throttle()
     Slows down the simulation for display purposes.
   ============================================================== */
void throttle (struct life_t * life) {
    unsigned int delay;
    int t = life->throttle;

    if (life->do_display && t != -1) {
        delay = 1000000 * 1 / t;
        usleep(delay);
    }
}

/* ==============================================================
   allocate_grids()
     Allocates memory for the LOCAL grid (nrows+2 rows to include
     ghost rows, ncols+2 columns for column ghosts).

     NOTE: The first dimension is ROWS here (i = row index).
     This differs from the original code where i iterated over
     cols — we keep the same pointer-to-pointer layout but the
     decomposition is now along rows.
   ============================================================== */
void allocate_grids (struct life_t * life) {
    int i;
    int ncols = life->ncols;
    int nrows = life->nrows;          /* local */

    /* nrows+2: real rows + top ghost + bottom ghost */
    life->grid      = (int **) malloc(sizeof(int *) * (nrows + 2));
    life->next_grid = (int **) malloc(sizeof(int *) * (nrows + 2));

    for (i = 0; i < nrows + 2; i++) {
        /* ncols+2: real cols + left ghost + right ghost */
        life->grid[i]      = (int *) malloc(sizeof(int) * (ncols + 2));
        life->next_grid[i] = (int *) malloc(sizeof(int) * (ncols + 2));
    }
}

/* ==============================================================
   init_grids()
     Initialises the local grid.

     When reading from a file, rank 0 reads the full grid and
     distributes each rank's rows via MPI_Send/MPI_Recv.
     When using a random grid, every rank initialises its own
     local portion independently (with a unique seed per rank).
   ============================================================== */
void init_grids (struct life_t * life) {
    int i, j;
    int rank  = life->rank;
    int size  = life->size;
    int ncols = life->ncols;
    int nrows = life->nrows;          /* local */

    allocate_grids(life);

    /* Zero-initialise (DEAD) including ghost rows. */
    for (i = 0; i < nrows + 2; i++)
        for (j = 0; j < ncols + 2; j++) {
            life->grid[i][j]      = DEAD;
            life->next_grid[i][j] = DEAD;
        }

    if (life->infile != NULL) {
        /* ----- File-based initialisation -----
         * Rank 0 reads the entire file and sends each rank its
         * rows.  Alive cell coordinates are broadcast as pairs.
         */
        if (rank == 0) {
            FILE * fd;
            int file_nrows, file_ncols;

            if ((fd = fopen(life->infile, "r")) == NULL) {
                perror("Failed to open input file");
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }

            if (fscanf(fd, "%d %d\n", &file_ncols, &file_nrows) == EOF) {
                printf("File must define grid dimensions!\nExiting.\n");
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            }

            /*
             * Read all alive cell coordinates.  For each cell,
             * determine which rank owns that row and send a
             * two-integer message {local_row, col} to that rank.
             * A sentinel {-1,-1} signals end-of-data.
             */
            int cell[2];
            int global_row, col;
            while (fscanf(fd, "%d %d\n", &global_row, &col) != EOF) {
                /* Determine owner rank and local row index. */
                int base   = life->global_nrows / size;
                int extras = life->global_nrows % size;
                int owner, local_row;

                /* rows 1..global_nrows are 1-based in the file */
                int gr = global_row - 1; /* 0-based global index */
                if (gr < extras * (base + 1)) {
                    owner     = gr / (base + 1);
                    local_row = (gr % (base + 1)) + 1;
                } else {
                    int adj = gr - extras * (base + 1);
                    owner     = extras + adj / base;
                    local_row = (adj % base) + 1;
                }

                if (owner == 0) {
                    /* Rank 0 sets its own grid directly. */
                    life->grid[local_row][col]      = ALIVE;
                    life->next_grid[local_row][col] = ALIVE;
                } else {
                    cell[0] = local_row;
                    cell[1] = col;
                    MPI_Send(cell, 2, MPI_INT, owner, 10, MPI_COMM_WORLD);
                }
            }
            fclose(fd);

            /* Send sentinel to all other ranks. */
            cell[0] = -1; cell[1] = -1;
            for (i = 1; i < size; i++)
                MPI_Send(cell, 2, MPI_INT, i, 10, MPI_COMM_WORLD);

        } else {
            /* Non-zero ranks receive their alive cells. */
            MPI_Status status;
            int cell[2];
            for (;;) {
                MPI_Recv(cell, 2, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
                if (cell[0] == -1) break;
                life->grid[cell[0]][cell[1]]      = ALIVE;
                life->next_grid[cell[0]][cell[1]] = ALIVE;
            }
        }

    } else {
        /* ----- Random initialisation -----
         * Each rank independently randomises its own local rows.
         * Seeds differ so the combined grid has good entropy.
         */
        srandom(time(NULL) + rank * 1000);
        randomize_grid(life, INIT_PROB);
    }
}

/* ==============================================================
   write_grid()
     Only rank 0 writes output.  All other ranks send their
     local rows to rank 0, which reassembles and writes the file.
   ============================================================== */
void write_grid (struct life_t * life) {
    int i, j;
    int rank  = life->rank;
    int size  = life->size;
    int ncols = life->ncols;
    int nrows = life->nrows;          /* local */

    if (life->outfile == NULL) return;

    if (rank == 0) {
        FILE * fd;
        if ((fd = fopen(life->outfile, "w")) == NULL) {
            perror("Failed to open output file");
            return;
        }

        fprintf(fd, "%d %d\n", life->global_nrows, ncols);

        /* Write rank 0's own rows. */
        for (i = 1; i <= nrows; i++)
            for (j = 1; j <= ncols; j++)
                if (life->grid[i][j] != DEAD)
                    fprintf(fd, "%d %d\n", i + life->row_offset, j);

        /* Receive and write each other rank's rows. */
        for (int src = 1; src < size; src++) {
            int remote_nrows, remote_offset;
            MPI_Status status;
            MPI_Recv(&remote_nrows,  1, MPI_INT, src, 20, MPI_COMM_WORLD, &status);
            MPI_Recv(&remote_offset, 1, MPI_INT, src, 21, MPI_COMM_WORLD, &status);

            /* Receive a flat buffer: (row, col) pairs, terminated by (-1,-1). */
            int pair[2];
            for (;;) {
                MPI_Recv(pair, 2, MPI_INT, src, 22, MPI_COMM_WORLD, &status);
                if (pair[0] == -1) break;
                /* pair[0] is the local row; convert to global 1-based. */
                fprintf(fd, "%d %d\n", pair[0] + remote_offset, pair[1]);
            }
        }

        fclose(fd);

    } else {
        /* Non-zero ranks: send metadata then alive cells. */
        MPI_Send(&nrows,           1, MPI_INT, 0, 20, MPI_COMM_WORLD);
        MPI_Send(&life->row_offset,1, MPI_INT, 0, 21, MPI_COMM_WORLD);

        int pair[2];
        for (i = 1; i <= nrows; i++) {
            for (j = 1; j <= ncols; j++) {
                if (life->grid[i][j] != DEAD) {
                    pair[0] = i; pair[1] = j;
                    MPI_Send(pair, 2, MPI_INT, 0, 22, MPI_COMM_WORLD);
                }
            }
        }
        /* Sentinel. */
        pair[0] = -1; pair[1] = -1;
        MPI_Send(pair, 2, MPI_INT, 0, 22, MPI_COMM_WORLD);
    }
}

/* ==============================================================
   free_grids()
     Frees the local grid memory.
   ============================================================== */
void free_grids (struct life_t * life) {
    int i;
    int nrows = life->nrows;          /* local */

    for (i = 0; i < nrows + 2; i++) {
        free(life->grid[i]);
        free(life->next_grid[i]);
    }

    free(life->grid);
    free(life->next_grid);
}

/* ==============================================================
   rand_double() — unchanged
   ============================================================== */
double rand_double () {
    return (double) random() / (double) RAND_MAX;
}

/* ==============================================================
   randomize_grid() — unchanged logic; operates on local nrows
   ============================================================== */
void randomize_grid (struct life_t * life, double prob) {
    int i, j;
    int ncols = life->ncols;
    int nrows = life->nrows;          /* local */

    #pragma omp parallel for private(j)
    for (i = 1; i <= nrows; i++)
        for (j = 1; j <= ncols; j++)
            if (rand_double() < prob)
                life->grid[i][j] = ALIVE;
}

/* ==============================================================
   cleanup()
     Write output, free memory, and finalise MPI.
   ============================================================== */
void cleanup (struct life_t * life) {
    write_grid(life);
    free_grids(life);

    if (life->do_display)
        free_video(life);

    MPI_Finalize();
}

/* ==============================================================
   usage() — unchanged
   ============================================================== */
void usage () {
    printf("\nUsage: Life [options]\n");
    printf("  -c|--columns number   Number of columns in grid. Default: %d\n", DEFAULT_SIZE);
    printf("  -r|--rows number      Number of rows in grid. Default: %d\n", DEFAULT_SIZE);
    printf("  -g|--gens number      Number of generations to run. Default: %d\n", DEFAULT_GENS);
    printf("  -i|--input filename   Input file. See README for format. Default: none.\n");
    printf("  -o|--output filename  Output file. Default: none.\n");
    printf("  -h|--help             This help page.\n");
    printf("  -t[N]|--throttle[=N]  Throttle display to N generations/second. Default: %d\n",
        DEFAULT_THROTTLE);
    printf("  -x|--display          Use a graphical display.\n");
    printf("  --no-display          Do not use a graphical display.\n");
    printf("     Default: %s\n", (DEFAULT_DISP ? "do display" : "no display"));
    printf("\nSee README for more information.\n\n");

    exit(EXIT_FAILURE);
}

/* ==============================================================
   parse_args() — unchanged
   ============================================================== */
void parse_args (struct life_t * life, int argc, char ** argv) {
    int opt       = 0;
    int opt_index = 0;
    int i;

    for (;;) {
        opt = getopt_long(argc, argv, opts, long_opts, &opt_index);
        if (opt == -1) break;

        switch (opt) {
            case 0:
                if (strcmp("no-display", long_opts[opt_index].name) == 0)
                    life->do_display = false;
                break;
            case 'c':
                life->ncols = strtol(optarg, (char **) NULL, 10);
                break;
            case 'r':
                life->nrows = strtol(optarg, (char **) NULL, 10);
                break;
            case 'g':
                life->generations = strtol(optarg, (char **) NULL, 10);
                break;
            case 'x':
                life->do_display = true;
                break;
            case 'i':
                life->infile = optarg;
                break;
            case 'o':
                life->outfile = optarg;
                break;
            case 't':
                if (optarg != NULL)
                    life->throttle = strtol(optarg, (char **) NULL, 10);
                else
                    life->throttle = DEFAULT_THROTTLE;
                break;
            case 'h':
            case '?':
                usage();
                break;
            default:
                break;
        }
    }

    /* Backwards-compatible positional argument parsing. */
    if (optind == 1) {
        if (argc > 1) life->nrows       = strtol(argv[1], (char **) NULL, 10);
        if (argc > 2) life->ncols       = strtol(argv[2], (char **) NULL, 10);
        if (argc > 3) life->generations = strtol(argv[3], (char **) NULL, 10);
        if (argc > 4) life->do_display  = strtol(argv[4], (char **) NULL, 10);
    }
}

#endif
