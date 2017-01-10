#include "ParallelTask.h"
#include <utility>

template<class... TASK>
class MPIntf : public TASK...
{
       	int world_rank, world_size;

public:
	MPIntf(const TASK&...);
	~MPIntf();

		// accessor methods
	int rank() const	{	return world_rank;	}
	int size() const	{	return world_size;	}

        template<class T>   MPIntf& operator<<(const T&);	// send built-in types
        template<class T, template<class> class CT>   MPIntf& operator<<(const CT<T>&);	// CT - container type ( vector, list, etc. )

        template<class T>   MPIntf& operator>>(T &v);
           
	template<class T> struct runtask
	{
	    T val;
	    runtask(const T &t) : 	val{t}	{}

	    operator T() const	{	return val;	}
	};


        template<class T> MPIntf& operator>>(const runtask<T> &t)
        {
		if(world_rank == 1)
			(*this)(t);
                return *this;
        }
};

#include "MPIntf.cpp"
