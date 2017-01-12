#include "ParallelTask.h"
#include <memory>

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

		// send
        template<class T>   MPIntf& operator<<(const T&);	// send built-in types
        template<class T, template<class, class> class CT>   MPIntf& operator<<(const CT<T, std::allocator<T>>&);	// CT - container type ( vector, list, etc. )

		// receive
        template<class T>   MPIntf& operator>>(T &v);
        template<class T, template<class, class> class CT>   MPIntf& operator>>(CT<T, std::allocator<T>>&);
           
	template<template<template<class, class> class CT, class T> class ProtoType, class T, template<class, class = std::allocator<T>> class CT>
	struct Apply
	{
		ProtoType<CT, T> fn;
		CT<T> vals;

		Apply(ProtoType<CT, T> fn, const CT<T> &c) : fn{fn}, vals{c}	{}
	};

	template<class T> struct runtask
	{
	    T val;
	    runtask(const T &t) : 	val{t}	{}

	    operator T() const	{	return val;	}
	};

	template<class T, template<class,class> class CT> struct runcontainertask
	{
		CT<T, std::allocator<T>> c;
		runcontainertask(const CT<T, std::allocator<T>> &c) : c{c}	{}
	};

	template<template<template<class, class> class CT, class T> class ProtoType, class T, template<class, class = std::allocator<T>> class CT>
	MPIntf& operator>>(const Apply<ProtoType, T, CT> &a)
	{
		if(world_rank != 0)	a.fn(a.vals);		
		return *this;
	}

        template<class T> MPIntf& operator>>(const runtask<T> &t)
        {
		if(world_rank == 1)
			(*this)(t);
                return *this;
        }

	template<class T, template<class, class = std::allocator<T>> class CT> MPIntf& operator>>(const runcontainertask<T, CT> &t)
	{
		if(world_rank == 1)	(*this)(t.c);
		return *this;
	}
};

#include "MPIntf.cpp"
