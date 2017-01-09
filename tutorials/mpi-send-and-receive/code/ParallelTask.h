#include <iostream>
#include <tuple>

namespace CSE856
{
	using std::tuple;

	template<template<class> class prototype, class... args>
	class FunctCall
	{
		prototype<tuple<args...>> fn;

		public:
			FunctCall(prototype<tuple<args...>> f) : fn{f}  {}

		        virtual void operator()(const tuple<args...>& a)	{       fn(a);  }
	};

	
	template<class T>	void PrintValue(tuple<T> v)
	{
    		std::cout << "Process 1 received number " << std::get<0>(v) << " from process 0 - tuple size: " << std::tuple_size<decltype(v)>::value << std::endl;
	}
	template<class T1, class T2>	void Print2Values(tuple<T1,T2> v)
	{
    		std::cout << "\nProcess 1 received number " << std::get<0>(v) << " from process 0 - tuple size: " << std::tuple_size<decltype(v)>::value << std::endl;
    		std::cout << "Process 1 received second number " << std::get<1>(v) << " from process 0 - tuple size: " << std::tuple_size<decltype(v)>::value << std::endl << std::endl;
	}

	template<class T> using TaskFunct = void (*)(T);
}

