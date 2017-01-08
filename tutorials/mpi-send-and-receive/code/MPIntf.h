#include "ParallelTask.h"
#include <utility>

namespace CSE856
{
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
            
            template<class... args> struct runtask
            {
		std::tuple<args...> params;
                runtask(const args&... a) : params{a...}	{}
            };
            

            template<class... args> MPIntf& operator>>(const runtask<args...> &t)
            {
		if(world_rank == 1)
			(*this)(std::get<0>(t.params));   
                return *this;
            }
	};

}

#include "MPIntf.cpp"
