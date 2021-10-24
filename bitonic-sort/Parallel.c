#include "Parallel.h"

int readVector(int *vector, int MAX_SIZE, int rank)  
{
	// Open the file (.dat - Binary)
	FILE *entrada = fopen("Entrada.dat", "rb");

	int i = 0;

	while(!feof(entrada))
	{
		if (fread(&vector[i], sizeof(int), 1, entrada))
		{
			if (feof(entrada))
				break;
		}
		i++;
	}

	fclose(entrada);
}

int *compareFunc(int *localList, int size, int partner, int process, int rank, MPI_Comm Comm, int setCompare)
{
	int *partnerList;

	partnerList = (int *)malloc(size * sizeof(int));

	// Send your list and Recieve the list to the partner
	MPI_Sendrecv(localList, size, MPI_INT, partner, 0, partnerList, size, MPI_INT, partner, 0, Comm, MPI_STATUS_IGNORE);

    printf("Rank(%d) Trocou com Partner (%d)\n", rank, partner);
    //printf("(%d)  %d %d -- %d %d \n", rank, localList[0], localList[size - 1], partnerList[0], partnerList[size - 1]);

	if (setCompare)
		localList = mergeLow(localList, partnerList, size);
	else
		localList = mergeHigh(localList, partnerList, size);

    //printf("(%d)  %d %d \n", rank, localList[0], localList[size - 1]);
	free(partnerList);

	return localList;
}

int *mergeLow(int *localList, int *partnerList, int size)
{
	int k = 0, j = 0;

	int *mergeVector = (int *)malloc(size * sizeof(int));

	// Compare the frist number of each list
	// pick the lowest and put in the new vector
	// Go to next element of that list
	for (int i = 0; i < size; i++) 
	{
	    if (localList[j] < partnerList[k]) 
			mergeVector[i] = localList[j++];
	    else 
	    	mergeVector[i] = partnerList[k++];
	}

	return mergeVector;
}

int *mergeHigh(int *localList, int *partnerList, int size)
{
	int k = (size - 1), j = (size - 1);

	int *mergeVector = (int *)malloc(size * sizeof(int));

	// Compare the last number of each list
	// pick the highest and put in the new vector
	// Go to previous element of that list
	for (int i = (size - 1); i >= 0; i--) 
	{
	    if (localList[j] > partnerList[k])
	    	mergeVector[i] = localList[j--];
	    else
	    	mergeVector[i] = partnerList[k--];
	}

	return mergeVector;
}

int testSort(int* vector, int size)
{
    int pass = 1;

    for (int i = 1; i < size; i++) 
        if (vector[i] < vector[i - 1])
        	pass = 0;

    if (pass)
    	printf("Success\n");
    else
    	printf("Fail\n");

    return pass;
}

int compareAsc(const void * a, const void * b) 
{
    return ( *(int *)a - *(int *)b );
}


/* ULFM Crash Functions */

void saveArchive(int *localList, int size, int rank)
{
	char fileName[16];

	sprintf(fileName, "saveRank%d.dat", rank);
	FILE *saveFile = fopen(fileName, "wb");

    for (int i = 0; i < size; i++)
        fwrite(&localList[i], sizeof(int), 1, saveFile);

	fclose(saveFile);
}

void MPI_myBarrier(int rank, int process, MPI_Comm *comm)
{
	int error = MPI_Barrier(*comm);
}

void makeMap(int *map, int *deadProcess, int rank, int process, int dim)
{
    int rankPickFail, j;

    for (int i = 0; i < process; i++)
    {
        deadProcess[i] = -1;

        if(verifyProcess[i] % 2 == DEAD)
        {
            j = 1;

            while (((rankPickFail = findPartnerMap(i, j)) == -1) && (j <= dim))
                j++;

            map[i] = rankPickFail;

            if(rank == rankPickFail)
                addDeadProcess(deadProcess, process, i);
        }
        else
        {
            map[i] = i;

            /*if(rank == i)
                addDeadProcess(deadProcess, process, i);*/
        }
    }
}

int findPartnerMap(int rank, int round)
{
    int j = 0, partner;
    node_set* nodes; 

    nodes = cis(rank, round);
    while( (j < nodes->size) && (verifyProcess[nodes->nodes[j]] % 2 == DEAD))
            j++;

    if(j == nodes->size)
       partner = -1;
    else
        partner = nodes->nodes[j];

    set_free(nodes);

    return partner;
}

void addDeadProcess(int *vector, int process, int myRank)
{
    int i = 0;

    while((vector[i] != -1) && (i < process))
        i++;

    vector[i] = myRank;
}

int testPartner(int rank, int myPartner, int round)
{
    int pass = 0;

    for (int i = 0; i < round; i++)
    {
        int partner;

        partner = rank ^ (1 << i);

        if (partner == myPartner)
            pass = 1;
    }

    return pass; 
}

int *pickFile(int rank, int partner, int *localList, int size, int i, int j)
{
    int *copy, *saveList;

    copy = (int *)malloc(size * sizeof(int));
    saveList = (int *)malloc(size * sizeof(int));

    readFromFile(partner, saveList);

    if (((rank >> (i + 1)) % 2 == 0 && (rank >> j) % 2 == 0) || ((rank >> (i + 1)) % 2 != 0 && (rank >> j) % 2 != 0))
    {
        copyList(localList, copy, size);
        localList = mergeLow(localList, saveList, size);
        saveList = mergeHigh(saveList, copy, size);

        //printf("(%d)  %d %d \n", rank, localList[0], localList[size - 1]);
        //printf("(%d)  %d %d \n", partner, saveList[0], saveList[size - 1]);
    }
    else
    {
        copyList(saveList, copy, size);
        saveList = mergeLow(saveList, localList, size);
        localList = mergeHigh(localList, copy, size);

        //printf("(%d)  %d %d \n", rank, localList[0], localList[size - 1]);
        //printf("(%d)  %d %d \n", partner, saveList[0], saveList[size - 1]);
    }

    printf("Saving Archive.... Rank(%d)\n", rank);
    saveArchive(saveList, size, partner);
    //saveArchive(localList, size, rank);

    free(copy);
    free(saveList);

    return localList;
}

int testDead(int *deadProcess, int process)
{
    for (int i = 0; i < process; i++)
        if (deadProcess[i] != -1)
            return 1;

    return 0;
}

void removeDead(int *deadProcess, int partner, int process)
{
    for (int i = 0; i < process; i++)
        if (deadProcess[i] == partner)
            deadProcess[i] = -1;
}

void copyList(int *vector, int *vector2, int size)
{
    for (int i = 0; i < size; i++)
        vector2[i] = vector[i];
}

int readFromFile(int rank, int *vector)
{
    char fileName[16];
    sprintf(fileName, "saveRank%d.dat", rank);

    FILE *read = fopen(fileName, "rb");

    int i = 0;

    while(!feof(read))
    {
        if (fread(&vector[i], sizeof(int), 1, read))
        {
            if (feof(read))
                break;
        }
        i++;
    }

    fclose(read);

    return i;
}

void myGather(int *vector, int size, int process)
{
    int sizeTotal = 0;

    for (int i = 0; i < process; i++)
    {
        char fileName[16];
        sprintf(fileName, "saveRank%d.dat", i);

        FILE *read = fopen(fileName, "rb");

        for (int j = 0; j < size; j++)
        {
            fread(&vector[sizeTotal], sizeof(int), 1, read);
            sizeTotal++;
        }

        fclose(read);
    }
}

void insertFailures(int whichFail, int **vectorFail, int **roundFail, int process, int rank)
{
    int temp, i, j, *p, *r;

    p = NULL;
    r = NULL;

    srand(time(NULL));

    switch (whichFail)
    {
        case 0:

            break;
        case 1:

            r = (int *) malloc(1 * sizeof(int));
            p = (int *) malloc(1 * sizeof(int));

            *r = rand() % (int)log2(process);
            *p = rand() % process;

            break;
        case 2:

            r = (int *) malloc((int)(process / 2) * sizeof(int));
            p = (int *) malloc((int)(process / 2) * sizeof(int));

            for(i = 0; i < (process / 2); i++)
            {
                r[i] = rand() % (int)log2(process);

                do{
                    temp = rand() % process;

                    j = 0;
                    while( (temp != p[j]) && (j <= i) )
                        j++;

                }while(j < i);

                p[i] = temp;
            }

            break;
        case 3:

            r = (int *) malloc((int)(process - 1) * sizeof(int));
            p = (int *) malloc((int)(process -1 ) * sizeof(int));

            for(i = 0; i < (process - 1); i++)
            {
                r[i] = rand() % (int)log2(process);

                do{
                    temp = rand() % process;

                    j = 0;
                    while( (temp != p[j]) && (j <= i) )
                        j++;

                }while(j < i);

                p[i] = temp;
            }

            break;
    }

    *vectorFail = p;
    *roundFail = r;
}

void applyFailures(int whichFail, int *vectorFail, int *roundFail, int process, int rank, int round)
{
    int i;

    switch (whichFail)
    {
        case 0:

            break;
        case 1:

            if(round== *roundFail)
               killRank(rank, *vectorFail);

            break;
        case 2:
            for(i = 0; i < (process / 2); i++)
                if(round == roundFail[i])
                    killRank(rank, vectorFail[i]);

            break;
        case 3:
            for(i = 0; i < (process - 1); i++)
                if(round == roundFail[i])
                    killRank(rank, vectorFail[i]);

            break;
    }
}

void killRank(int rank, int processFail)
{
  return;
}

void removeFile(int process)
{
    for (int i = 0; i < process; i++)
    {
        char fileName[15];

        sprintf(fileName,"saveRank%d.dat", i);

        remove(fileName);
    }
}