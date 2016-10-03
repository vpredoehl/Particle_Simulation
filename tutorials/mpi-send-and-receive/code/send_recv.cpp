// Author: Wes Kendall
// Copyright 2011 www.mpitutorial.com
// This code is provided freely with the tutorials on mpitutorial.com. Feel
// free to modify it for your own use. Any distribution of the code must
// either provide a link to www.mpitutorial.com or keep this header intact.
//
// MPI_Send, MPI_Recv example. Communicates the number -1 from process 0
// to process 1.
//
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
 
#include "MPIntf.h"

int main(int argc, char** argv) {
	CSE856::MPIntf p;

	int world_size = p.size();
	int world_rank = p.rank();

printf("World size: %i\n", world_size);
printf("World rank: %i\n", world_rank);

  // We are assuming at least 2 processes for this task
  //if (world_size < 2) {
    ////fprintf(stderr, "World size must be greater than 1 for %s\n", argv[0]);
    ////MPI_Abort(MPI_COMM_WORLD, 1);
  //}

  int number;
  if (world_rank == 0) {
    // If we are rank 0, set the number to -1 and send it to process 1
    number = -1;
    MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
  } else if (world_rank == 1) {
    MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process 1 received number %d from process 0\n", number);
  }
}
