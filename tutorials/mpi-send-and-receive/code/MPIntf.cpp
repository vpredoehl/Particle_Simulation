#include <vector>
#include <typeinfo>
#include <mpi.h>


// define template operators here to remove clutter from interface
template<class... TASK> template<class T, template<class> class CT>
CSE856::MPIntf<TASK...>& CSE856::MPIntf<TASK...>::operator<<(const CT<T> &v)
{
	if(typeid(v) == typeid(CommType<T>))
	{
       		if(world_rank == 0)    MPI_Send(&v, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	}
	return *this;
}

template<class... TASK> template<class T>
CSE856::MPIntf<TASK...>& CSE856::MPIntf<TASK...>::operator>>(T &v)
{
    if(world_rank != 0)
    {
        MPI_Recv(&v, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    return *this;
}

template<class... TASK>
CSE856::MPIntf<TASK...>::MPIntf(const TASK&... t)
	: TASK(t)...
{
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
}

template<class... TASK>
CSE856::MPIntf<TASK...>::~MPIntf()
{
  MPI_Finalize();
}

