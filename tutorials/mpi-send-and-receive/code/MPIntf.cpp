
#include <mpi.h>
#include "MPIntf.h"


//using namespace CSE856;

CSE856::MPIntf::MPIntf()
{
    parallelTask = nullptr;
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
}

CSE856::MPIntf::~MPIntf()
{
  MPI_Finalize();
}

