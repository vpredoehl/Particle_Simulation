#include "ParallelTask.h"

template<class T> using PrintTaskProto = void (*)(T);


struct PrintValue : protected Task<int>
{
	void operator()(int v) override	{	std::cout << "Object:\t\t Process 1 received number " << v << " from process 0\n";	}
};

template<class T>	void PrintValue(T v)
{
	std::cout << "PrintValue:\t Process 1 received number " << v << " from process 0\n";
}

template<class T1, class T2>	void Print2Values(tuple<T1,T2> v)
{
	std::cout << "\nProcess 1 received number " << std::get<0>(v) << " from process 0 - tuple size: " << std::tuple_size<decltype(v)>::value << std::endl;
    	std::cout << "Process 1 received second number " << std::get<1>(v) << " from process 0 - tuple size: " << std::tuple_size<decltype(v)>::value << std::endl << std::endl;
}

