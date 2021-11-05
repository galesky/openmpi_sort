#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>
#include "Parallel.c"
#include "Parallel.h"
#include <stdbool.h>

int main(int argc, char** argv)
{
	int rank, process, size, *localList, *vector, dim, partner, MAX_SIZE; // Variables from Bitonic Sort Original
	double startTime, endTime;

    // New communicators
    MPI_Comm commSort, commBackup;

    srand(time(NULL));

    if (argc < 3) 
    {
        printf("Missing arguments\n %s # of elements\n", argv[0]);
        return 0;
    }

    printf("Starting MPI\n");

        
    // Inicialization
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);      // Each process ID
	MPI_Comm_size(MPI_COMM_WORLD, &process);   // Number total of process

    // Set the error handler
    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    // Duplicate the original Communicator
    MPI_Comm_dup(MPI_COMM_WORLD, &commSort);    // Communicator to be used to sort
    MPI_Comm_dup(MPI_COMM_WORLD, &commBackup);  // Backup communicator

    // Start data
    MAX_SIZE = atoi(argv[1]);       // Input size
    size = MAX_SIZE / process;      // Size of local data
    numFailures = 0;                // Number of Failures

    // Cube dimension
    dim = (int )(log(process) / log(2));
    printf("Allocating list memory\n");
    // Allocate memory
    localList = (int *)malloc(size * sizeof(int));          // Local list for each process 
    verifyProcess = (int *)malloc(process * sizeof(int));   // Vector who say which process are stable
    
    
    // Set all process as stable
    for (int i = 0; i < process; i++)
        verifyProcess[i] = ALIVE;
    // Only MASTER read the file
	if(rank == MASTER)
    {
        // Allocate memory for vector
        printf("MASTER: File mem vector\n");
		vector = (int *)malloc(MAX_SIZE * sizeof(int));
        printf("MASTER: File read started\n");
        // Read the numbers of a file(.dat)
        readVector(vector, MAX_SIZE, rank);
        printf("MASTER: File read completed\n");
        printf("MASTER: First vector item (%d)",vector[0]);
        printf("MASTER: Middle vector item (%d)",vector[50000]);
        printf("MASTER: First vector item (%d)",vector[99999]);

    }   
    // Blocks until MASTER process have finished read the data
    MPI_Barrier(commBackup); 
    printf("Post Barrier(%d)\n", rank);
    // Divide data between all processes
 	MPI_Scatter(vector, size, MPI_INT, localList, size, MPI_INT, 0, commSort);  

    // Start Sort - Time(s)
    startTime = MPI_Wtime();

    // Local sort
 	qsort(localList, size, sizeof(int), compareAsc);
    
    MPI_Barrier(commBackup); 

    for (int i = 0; i < dim; i++) 
    {
        printf("Rodada %d rank (%d)\n", i, rank);
        for (int j = i; j >= 0; j--) 
        {   
            printf("Inicio j (%d)\n", rank);
            // Find the process partner
            partner = rank ^ (1 << j); 

            if (rank != partner)
            { 
                if (testPartner(rank, partner, dim))
                {
                    // Window_id = Most Significant [dim - (i + 1)] bits of process
                    // Window_id is even AND jth bit of process is 0 then CompareLow OR Window_id is odd AND jth bit of process is 1 then CompareHigh
                    if (((rank >> (i + 1)) % 2 == 0 && (rank >> j) % 2 == 0) || ((rank >> (i + 1)) % 2 != 0 && (rank >> j) % 2 != 0))
                        localList = compareFunc(localList, size, partner, process, rank, commSort, ASC);
                    else
                        localList = compareFunc(localList, size, partner, process, rank, commSort, DESC);
                    // Test if need to take the place of someone
                }
                else
                {
                    // Window_id = Most Significant [dim - (i + 1)] bits of process
                    // Window_id is even AND jth bit of process is 0 then CompareLow OR Window_id is odd AND jth bit of process is 1 then CompareHigh
                    if (((rank >> (i + 1)) % 2 == 0 && (rank >> j) % 2 == 0) || ((rank >> (i + 1)) % 2 != 0 && (rank >> j) % 2 != 0))
                    {
                        localList = compareFunc(localList, size, partner, process, rank, commSort, ASC);
                        // Test if need to take the place of someone
                    }
                    else
                    {
                        localList = compareFunc(localList, size, partner, process, rank, commSort, DESC);
                    }
                }
            }
            else if(rank == partner)
            {
                // Find the process partner
                partner = rank ^ (1 << j);

                // If need take place of someone, but dont need exchange list with other process (rank == partner)
                localList = pickFile(rank, partner, localList, size, i, j);
                // Test if need to take the place of someone
            }

            printf("Saving Archive.... Rank(%d)\n", rank);
            // Save local data in file
            saveArchive(localList, size, rank);

            // Blocks until all processes have finished the round
            MPI_Barrier(commBackup);
        }
    }

    printf("Saving Archive.... Rank(%d)\n", rank);
    // Save local data in file
    saveArchive(localList, size, rank);

    // Blocks until all processes have finished the local sort
    MPI_Barrier(commBackup);

    // End sort
    endTime = MPI_Wtime();

    // Blocks until all processes have finished the sort
    MPI_Barrier(commBackup);

    // Show the time
    if (rank == MASTER){
        bool isSorted = true;
        
        for (int i = 1; i < MAX_SIZE; i++){
            printf("Item : %d\n", localList[i]);
            if (localList[i] < localList[i-1]) {
                isSorted = false;
            }
        }
        if (isSorted == false) {
            printf("Sort failed : %f\n", endTime - startTime);
        } else {
            printf("Sort sucessfull  ! \n");
            printf("Sort Time: %f\n", endTime - startTime);
        }
    }
    if (rank == MASTER)
        free(vector);

    free(localList);
    
	MPI_Finalize();

	return 0;
}