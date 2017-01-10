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

	// function object to call task
template<class T>
struct Task
{
	virtual void operator()(T) = 0;
};


#endif

