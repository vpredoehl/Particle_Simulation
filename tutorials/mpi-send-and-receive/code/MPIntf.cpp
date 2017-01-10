#include <vector>
#include <typeinfo>
#include <mpi.h>


template<class... TASK> template<class T, template<class> class CT>
MPIntf<TASK...>& MPIntf<TASK...>::operator<<(const CT<T> &v)	// CT - container type ( vector, list, etc. )
{
	return *this;
}

template<class... TASK> template<class T>
MPIntf<TASK...>& MPIntf<TASK...>::operator<<(const T &v)
{
       	if(world_rank == 0)
	{
	    if(typeid(T) == typeid(int))	MPI_Send(&v, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	    else if(typeid(T) == typeid(float))	MPI_Send(&v, 1, MPI_FLOAT, 1, 0, MPI_COMM_WORLD);
	}
	    
	return *this;
}


template<class... TASK> template<class T>
MPIntf<TASK...>& MPIntf<TASK...>::operator>>(T &v)
{
    if(world_rank != 0)
    {
        MPI_Recv(&v, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    return *this;
}

template<class... TASK>
MPIntf<TASK...>::MPIntf(const TASK&... t)
	: TASK(t)...
{
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
}

template<class... TASK>
MPIntf<TASK...>::~MPIntf()
{
  MPI_Finalize();
}

