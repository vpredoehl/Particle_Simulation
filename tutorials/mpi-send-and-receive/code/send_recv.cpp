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

void PrintValue(int v)
{
    std::cout << "Process 1 received number " << v << " from process 0\n";
}

int main(int argc, char** argv) {
	CSE856::MPIntf mp;

	int world_size = mp.size();
	int world_rank = mp.rank();

printf("World size: %i\n", world_size);
printf("World rank: %i\n", world_rank);

  // We are assuming at least 2 processes for this task
  //if (world_size < 2) {
    ////fprintf(stderr, "World size must be greater than 1 for %s\n", argv[0]);
    ////MPI_Abort(MPI_COMM_WORLD, 1);
  //}

    CSE856::MPIntf::CommType<int> number;

    number = -1;
    mp << number;
    mp >> CSE856::MPIntf::setparalleltask<int>(PrintValue) >>  number;

    return 0;
}
