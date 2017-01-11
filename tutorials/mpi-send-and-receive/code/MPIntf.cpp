#include <vector>
#include <typeinfo>
#include <mpi.h>


template<class... TASK> template<class T, template<class, class> class CT>
MPIntf<TASK...>& MPIntf<TASK...>::operator<<(const CT<T, std::allocator<T>> &v)	// CT - container type ( vector, list, etc. )
{
	unsigned long numItems = v.size();

	(*this) << numItems;
	for_each(v.begin(), v.end(), [this](T val)	{	(*this) << val;	});
	return *this;
}

template<class... TASK> template<class T>
MPIntf<TASK...>& MPIntf<TASK...>::operator<<(const T &v)
{
       	if(world_rank == 0)
	{
	    if(typeid(T) == typeid(int))	MPI_Send(&v, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	    else if(typeid(T) == typeid(float))	MPI_Send(&v, 1, MPI_FLOAT, 1, 0, MPI_COMM_WORLD);
	    else if(typeid(T) == typeid(unsigned long))	MPI_Send(&v, 1, MPI_UNSIGNED_LONG, 1, 0, MPI_COMM_WORLD);
	}
	    
	return *this;
}

template<class... TASK> template<class T, template<class, class Alloc = std::allocator<T>> class CT>
MPIntf<TASK...>& MPIntf<TASK...>::operator>>(CT<T> &v)	
{
	unsigned long numItems;
	T item;

	if(world_rank != 0)
	{
		(*this) >> numItems;
		for(int i = 0; i<numItems; ++i)
		{
			(*this) >> item;
			v.push_back(item);
		}
	}
	return *this;
}

template<class... TASK> template<class T>
MPIntf<TASK...>& MPIntf<TASK...>::operator>>(T &v)
{
    if(world_rank != 0)
    {
        if(typeid(T) == typeid(int))	MPI_Recv(&v, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	else if(typeid(T) == typeid(float))	MPI_Recv(&v, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	else if(typeid(T) == typeid(unsigned long))	MPI_Recv(&v, 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
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

