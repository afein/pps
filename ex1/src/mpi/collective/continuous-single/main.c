/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : main.c
 * Creation Date : 30-10-2012
 * Last Modified : Thu 22 Nov 2012 06:07:13 PM EET
 * Created By : Greg Liras <gregliras@gmail.com>
 * Created By : Alex Maurogiannis <nalfemp@gmail.com>
 _._._._._._._._._._._._._._._._._._._._._.*/

#include <mpi.h> 
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "common.h"


/* Returns the index at which the k-th row is located for a given rank */
int get_short_index(int k, int rank, int *displs, int *counts) {
    int result = -1;
    if (k <= (displs[rank] + counts[rank] - 1)) {
        if (k >= displs[rank]) {
            result = k - displs[rank]; 
        } else {
            result = 0;
        }
    }
    debug("for k: %d and rank: %d, i think the index is %d\n",k,rank,result);
    return result;
}

/* Returns the rank of the broadcaster given the previous one */
int get_bcaster(int *ccounts, int bcaster, int max_rank) 
{
    int result;
    if (ccounts[bcaster] > 0 ){
        result =  bcaster;
    } 
    else {
        result = bcaster + 1;
    }
    result = MIN(max_rank, result);
    ccounts[result]--;
    ccounts[result] = MAX(ccounts[result], 0);
    return result;
}

/* Returns the displacements table in rows */
void get_displs(int *counts, int max_rank, int *displs) 
{
    int j;
    displs[0] = 0;
    for (j = 1; j < max_rank ; j++) {
        displs[j] = displs[j - 1] + counts[j - 1];
    }
}

/*      performs the calculations for a given set of rows.
 *      Each thread is assigned a continuous quantity of rows
 */
void process_rows(int k, int rank, int N, int *counts, int *ccounts, \
        int *displs, double **Ap2D, double *Ak)
{
    int j, w;
    double l;
    int start = counts[rank] - ccounts[rank];
    for (w = start; w < (start + counts[rank]) && w < N && w >= 0; w++) {
        l = Ap2D[w][k] / Ak[k];
        for (j = k; j < N; j++) {
            Ap2D[w][j] = Ap2D[w][j] - l * Ak[j];
        }
    }
    
}

/*  distributes the rows in a continuous fashion */
void distribute_rows(int max_rank, int N, int *counts) 
{
    int j, k;
    int rows = N;

    /* Initialize counts */
    for (j = 0; j < max_rank ; j++) {
        counts[j] = (rows / max_rank);
    }

    /* Distribute the indivisible leftover */
    if (rows / max_rank != 0) {
        j = rows % max_rank;    
        for (k = 0; k < max_rank && j > 0; k++, j--) {
            counts[k] += 1;
        }
    } 
    else {
        for (k = 0; k < max_rank; k++) {
            counts[k] = 1;
        }
    }
}

int main(int argc, char **argv)
{
    int k;
    int N;
    int i;
    int rank;
    int max_rank;
    int workload;
    int *counts;
    int *displs;
    int *ccounts;
    int ret = 0;
    int bcaster = 0;
    double sec;
    double *A = NULL;
    double **A2D = NULL;
    double *Ap = NULL;
    double **Ap2D = NULL;
    double *Ak = NULL;
    FILE *fp = NULL;
    //MPI_Datatype row_type;

    usage(argc, argv);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);

    /* Root gets the matrix */
    if (rank == 0){
        Matrix *mat = get_matrix(argv[1], max_rank);
        N = mat->N;
        A = mat->A;
        A2D = appoint_2D(A, N, N);
    }
    /* And broadcasts N */
    //( cause we dont want others to know about A)
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    workload = (N / max_rank) + 1; // Max number of rows for each thread

    if (rank == 0){
        debug("A: \n");
#if main_DEBUG
        print_matrix_2d(N, N, A);
#endif
    }

    /* Allocations */
    Ak = malloc(N * sizeof(double));
    Ap = malloc(workload * N * sizeof(double));
    counts = malloc(max_rank * sizeof(int));
    displs = malloc(max_rank * sizeof(int));
    ccounts = malloc(max_rank * sizeof(int));

    /* Initializations */
    distribute_rows(max_rank, N, counts);
    get_displs(counts, max_rank, displs);
    memcpy(ccounts, counts, max_rank * sizeof(int));

    /* Scatter the table to each thread's Ap */
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Scatter(A, workload * N, MPI_DOUBLE, \
            Ap, workload * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    Ap2D = appoint_2D(Ap, N, N);
        /* Start Timing */
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
        sec = timer();
    }

    for (k = 0; k < N - 1 ; k++) {
        /* Find who owns the k-th row */
        bcaster = k / workload; //get_bcaster(ccounts, bcaster, max_rank);
            
        /* The broadcaster puts his k-th row in the Ak buffer */
        if (rank == bcaster) {
            //i = get_short_index(k, rank, displs, counts);
            i = k % workload;
            debug("k: %d bcaster is %d i: %d\n", k, bcaster, i);
            memcpy(Ak, Ap2D[i], N * sizeof(double));
        }

        /* Everyone receives the k-th row */
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(Ak, N, MPI_DOUBLE, bcaster, MPI_COMM_WORLD);

        /* Root collects all the broadcasts to fill the final matrix */
        if (rank == 0) {
            //debug("k: %d bcaster is %d i: %d\n", k, bcaster, i);
            memcpy(A2D[k], Ak, N * sizeof(double));
        }

        /* And off you go to work. */
        process_rows(k, rank, N, counts, ccounts, displs, Ap2D, Ak);
    }

    { /* This will collect the final data we need to root */
        /* Find who owns the k-th row */
        bcaster = max_rank - 1;
            
        /* The broadcaster puts his k-th row in the Ak buffer */
        if (rank == bcaster){
            memcpy(Ak, Ap2D[workload - 1], N * sizeof(double));
        }
        /* Everyone receives the k-th row */
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(Ak, N, MPI_DOUBLE, bcaster, MPI_COMM_WORLD);

        /* Root collects all the broadcasts to fill the final matrix */
        if (rank == 0) {
            memcpy(A2D[N-1], Ak, N * sizeof(double));
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
        sec = timer();
        printf("Calc Time: %lf\n", sec);
    }

    ret = MPI_Finalize();
    if(ret == 0) {
        debug("%d FINALIZED!!! with code: %d\n", rank, ret);
    }
    else {
        debug("%d NOT FINALIZED!!! with code: %d\n", rank, ret);
    }

    if(rank == 0) {
        fp = fopen(argv[2], "w");
        fprint_matrix_2d(fp, N, N, A);
        fclose(fp);
        free(A);
        free(A2D);
    }
    free(Ap);
    free(Ap2D);
    free(Ak);
    free(displs);
    free(counts);
    free(ccounts);

    return 0;
}
