namespace CSE856
{
	template<template<class> class proto, class... args>
	using TaskProto = proto<args...>;

	template<template<class> class prototype, class... args>
	class FunctCall
	{
		prototype<args...> fn;

		public:
			FunctCall(prototype<args...> f) : fn{f}  {}

        virtual void operator()(const args&... a)	{       fn(a...);  }
	};

	
	void PrintValue(int v);

	template<class T> using PrintValueFunctType = void (*)(T);
}

