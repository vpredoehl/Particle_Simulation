#include <tuple>

namespace CSE856
{
	template<template<class> class proto, class... args>
	using TaskProto = proto<args...>;

	template<template<class> class prototype, class... args>
	class FunctCall
	{
		prototype<args...> fn;
		std::tuple<args...> params;

		public:
			FunctCall(prototype<args...> f) : fn{f}  {}

		        virtual void operator()(const args&... a)	{       fn(a...);  }
	};

	
	void PrintValue(int v);
	void PrintValue(float v);

	template<class T> using PrintValueFunctType = void (*)(T);
}

