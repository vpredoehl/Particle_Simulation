#ifndef __PARALLEL_TASK
#define __PARALLEL_TASK

#include <iostream>
#include <tuple>

using std::tuple;

	// forward task to C function
template<template<class> class ProtoType, class T>
class FunctCall
{
	ProtoType<T> fn = nullptr;

	public:
		FunctCall()	{}
		FunctCall(ProtoType<T> f) : fn{f}  {}

	        virtual void operator()(const T& a)	{       fn(a);  }
};

template<template<template<class, class> class CT, class T> class ProtoType, class T, template<class, class = std::allocator<T>> class CT>
class FunctCallContainer
{
	ProtoType<CT, T>	fn;

	public:
		FunctCallContainer(ProtoType<CT,T> f)	: fn{f}	{}

		virtual void operator()(const CT<T> &c)	{	fn(c);	}
};

	// function object to call task
template<class T>
struct Task
{
	virtual void operator()(T) = 0;
};


#endif

