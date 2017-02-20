#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define MASTER  0
#define RECEIVER 1

int main(int argc, char *argv[])
{
int numtasks, taskid,len;
char hostname[MPI_MAX_PROCESSOR_NAME];

MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Get_processor_name(hostname, &len);

int size;
int max = 100;
int interval = 1;

int* val = malloc(sizeof(int) * max);
FILE* fout = NULL;

if(taskid == MASTER){
  fout = fopen("results.txt", "w");
}

for(size = 1; size < max;size += interval){
  if(taskid == MASTER){
    double start,end;
    MPI_Status status;
    int msgtag;

    start = MPI_Wtime();
    MPI_Send(val, size, MPI_INT, RECEIVER, msgtag,MPI_COMM_WORLD);
    MPI_Recv(val, size, MPI_INT, RECEIVER, msgtag,MPI_COMM_WORLD,&status);
    end = MPI_Wtime();

    printf("%f seconds over %d items",end-start, size);
    fprintf(fout, "%d, %f\n",size, end-start);
  }
  else if(taskid == RECEIVER){
    MPI_Status status;
    int msgtag;

    MPI_Recv(val, size, MPI_INT, MASTER, msgtag,MPI_COMM_WORLD,&status);
    MPI_Send(val, size, MPI_INT, MASTER, msgtag,MPI_COMM_WORLD);
  }
}

if(taskid == MASTER){
  fclose(fout);
}

free(val);

MPI_Finalize();

}
