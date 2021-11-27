#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>
#include "Parallel.c"
#include "Parallel.h"

int main(int argc, char** argv)
{
	int rank, process, size, *localList, *vector, dim, partner, MAX_SIZE; // Variables from Bitonic Sort Original

	double startTime, endTime;

    // New communicators
    MPI_Comm commSort;

    srand(time(NULL));

    if (argc < 2)
    {
        printf("Missing arguments\n %s # of elements\n", argv[0]);
        printf("Please enter with the file size\n");
        return 0;
    }

    // Inicialization
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);      // Each process ID
	MPI_Comm_size(MPI_COMM_WORLD, &process);   // Number total of process


    // Duplicate the original Communicator
    MPI_Comm_dup(MPI_COMM_WORLD, &commSort);    // Communicator to be used to sort

    // Start data
    MAX_SIZE = atoi(argv[1]);       // Input size
    size = MAX_SIZE / process;      // Size of local data
    numFailures = 0;                // Number of Failures

    // Cube dimension
    dim = (int )(log(process) / log(2));

    // Allocate memory
    localList = (int *)malloc(size * sizeof(int));          // Local list for each process


    // Only MASTER read the file
	if(rank == MASTER)
    {
        // Allocate memory for vector
		vector = (int *)malloc(MAX_SIZE * sizeof(int));

        // Read the numbers of a file(.dat)
        readVector(vector, MAX_SIZE, rank);
    }

    // Blocks until MASTER process have finished read the data
    MPI_Barrier(commSort);

    // Divide data between all processes
 	MPI_Scatter(vector, size, MPI_INT, localList, size, MPI_INT, 0, commSort);

    // Start Sort - Time(s)
    startTime = MPI_Wtime();

    // Local sort
 	qsort(localList, size, sizeof(int), compareAsc);

    printf("Saving Archive.... Rank(%d)\n", rank);

    // Save local data in file
    //saveArchive(localList, size, rank);

    //MPI_Barrier(commSort);

    for (int i = 0; i < dim; i++)
    {
        printf("Rodada %d rank (%d)\n", i, rank);

        for (int j = i; j >= 0; j--)
        {
            printf("Inicio j (%d)\n", rank);

            // Find the process partner
            partner = rank ^ (1 << j);

            if (testPartner(rank, partner, dim))
            {
                // Window_id = Most Significant [dim - (i + 1)] bits of process
                // Window_id is even AND jth bit of process is 0 then CompareLow OR Window_id is odd AND jth bit of process is 1 then CompareHigh
                if (((rank >> (i + 1)) % 2 == 0 && (rank >> j) % 2 == 0) || ((rank >> (i + 1)) % 2 != 0 && (rank >> j) % 2 != 0))
                    localList = compareFunc(localList, size, partner, process, rank, commSort, ASC);
                else
                    localList = compareFunc(localList, size, partner, process, rank, commSort, DESC);

                    // Test if need to take the place of someone
                    //testIfNeedTakePlace(map, partner, rank, process, processSave, size, deadProcess, commSort, i, j);
            }
            else
            {
                // Window_id = Most Significant [dim - (i + 1)] bits of process
                // Window_id is even AND jth bit of process is 0 then CompareLow OR Window_id is odd AND jth bit of process is 1 then CompareHigh
                if (((rank >> (i + 1)) % 2 == 0 && (rank >> j) % 2 == 0) || ((rank >> (i + 1)) % 2 != 0 && (rank >> j) % 2 != 0))
                {
                    localList = compareFunc(localList, size, partner, process, rank, commSort, ASC);

                }
                else
                {
                    localList = compareFunc(localList, size, partner, process, rank, commSort, DESC);
                }
            }
        }
        MPI_Barrier(commSort);
    }

    printf("Saving Archive.... Rank(%d)\n", rank);

    // MPI_Gatther();

    // Save local data in file

    saveArchive(localList, size, rank);


    MPI_Barrier(commSort);

    // End sort
    endTime = MPI_Wtime();

    // Blocks until all processes have finished the sort
    MPI_Barrier(commSort);

    // Show the time
    if (rank == MASTER)
        printf("Sort Time: %f\n", endTime - startTime);

    if (rank == MASTER)
        free(vector);

    free(localList);

	MPI_Finalize();

	return 0;
}
