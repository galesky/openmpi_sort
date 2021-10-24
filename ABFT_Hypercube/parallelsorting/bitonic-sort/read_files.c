
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void lerDados(int rank, int *dados, int *tam);
void exibe(int *v, int rank, int quant);
void ring(int rank, int size, int *v, int quant, MPI_Comm comm_s);
void exibe2(int *v, int rank, int quant);

int main(int argc, char *argv[])
{
    int rank;
    int *numeros;
    int size;
    MPI_Comm world;
    int tam;

    if(argc!= 2){
        printf("Informar o tamanho total dos dados \n");
        printf("./exec <tamanho>");
        exit(0);

    }

    tam = atoi(argv[1]);

    MPI_Init(&argc, &argv);
    MPI_Comm_dup(MPI_COMM_WORLD, &world);
    MPI_Comm_rank(world, &rank);
    MPI_Comm_size(world, &size);

    /* error handler */
    MPI_Comm_set_errhandler(world, MPI_ERRORS_RETURN);

    numeros = (int*) malloc (sizeof(int) * tam);

    if(numeros == NULL){
        printf("mem problem\n");
        exit(0);
    }

    lerDados(rank, numeros, &tam);

    //printf("Rank %d: %d\n", rank, t);

    ring(rank, size, numeros, tam, world);

    //printf("Process %d finalizing\n", rank);

    free(numeros);
    MPI_Finalize();
    return 0;
}

void lerDados(int rank, int *dados, int *tam){
    FILE *file;
    char file_name[16];
    sprintf(file_name,"saveRank%d.dat", rank );
    int i = 0;

    if( (file = fopen(file_name, "rb")) == NULL){
        printf("arquivo não pode ser aberto %d\n", rank);
	*tam = 0;
        return;
    }
    while(!feof(file)){
        if( fread(&dados[i], sizeof(int), 1, file) != 1 ){
            if(feof(file)) break;
            printf("Erro de leitura %s", file_name);
        }
        i++;
    }
    *tam = i;
    fclose(file);
}
void exibe(int *v, int rank, int quant){
    int i;

    printf("Process %d |", rank);
    for(i = 0; i < quant; i++){
        printf("%d  ", v[i]);
    }
    printf("\nTOTAL: %d\n", i);
    printf("\n---------------------------------\n");

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

void exibe2(int *v, int rank, int quant){

    printf("Process %d | %d - %d | TOTAL: %d", rank, v[0], v[quant-1], quant);
    printf("\n---------------------------------\n");
    testSort(v, quant);
}

/*
  Um anel. O processo 0 envia para o processo 1 e assim por diante
  até o último receber.

  Usado para saida
*/
void ring(int rank, int size, int *v, int quant, MPI_Comm comm_s){
    int n = 0; // um sinal "pode imprimir".
    int tag = 80;

    if (rank == 0) {
        exibe2(v, rank, quant);
        MPI_Send( &n, 1, MPI_INT, rank + 1, tag, comm_s );

    }
    else {
        //printf("Process %d waiting\n", rank);
         MPI_Recv( &n, 1, MPI_INT, rank - 1, tag, comm_s, MPI_STATUS_IGNORE );
         exibe2(v, rank, quant);
         if (rank < size - 1)
             MPI_Send( &n, 1, MPI_INT, rank + 1, tag, comm_s );
    }
}

