#ifndef PARALLEL_H_INCLUDED
#define PARALLEL_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>
//#include <mpi-ext.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include "CIS.h"

//#define MASTER 0
#define ASC 1
#define DESC 0
#define ALIVE 0
#define DEAD 1

//int *verifyProcess, numFailures, MASTER = 0;
int numFailures, MASTER = 0;

/* Parallel Bitonic Sort Functions */

int readVector(int *vector, int MAX_SIZE, int rank);
int *compareFunc(int *localList, int size, int partner, int process, int rank, MPI_Comm Comm, int setCompare);

int *mergeLow(int *localList, int *partnerList, int size);
int *mergeHigh(int *localList, int *partnerList, int size);

int testSort(int *vector, int size);
int compareAsc(const void * a, const void * b);

/* ULFM Crash Functions */

void saveArchive(int *localList, int size, int rank);

void MPI_myBarrier(int rank, int process, MPI_Comm *comm);
void makeNewComm(int rank, int process, MPI_Comm *comm);
void repairComm(int process, MPI_Comm **comm);

void makeMap(int *map, int *deadProcess, int rank, int process, int dim);
int findPartnerMap(int rank, int round);
void addDeadProcess(int *vector, int process, int myRank);

int testPartner(int rank, int myPartner, int round);
void testIfNeedTakePlace(int *map, int partner, int rank, int process, int processSave, int size, int *deadProcess, MPI_Comm *commSort, int i, int j);
int *pickFile(int rank, int partner, int *localList, int size, int i, int j);
void copyList(int *vector, int *vector2, int size);
int testDead(int *deadProcess, int process);
void removeDead(int *deadProcess, int partner, int process);

int readFromFile(int rank, int *vector);
void myGather(int *vector, int size, int process);

void insertFailures(int whichFail, int **vectorFail, int **roundFail, int process, int rank);
void applyFailures(int whichFail, int *vectorFail, int *roundFail, int process, int rank, int round);
void killRank(int rank, int n);

void removeFile(int process);

#endif
