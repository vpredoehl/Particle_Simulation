#include "ParallelTask.h"
#include <utility>

namespace CSE856
{
    	template<class... TASK>
	class MPIntf : protected TASK...
	{
        	int world_rank, world_size;

	public:
		MPIntf(const TASK&...);
		~MPIntf();

			// accessor methods
		int rank() const	{	return world_rank;	}
		int size() const	{	return world_size;	}

                // wrapper class for built-in communication types
            template<class T> class CommType
            {
                T v;
            public:
            
                T value() const {   return v;   }
                operator T() const  {   return v;   }
                const T* operator&() const    {   return &v;  }
                T* operator&()    {   return &v;  }
                
                CommType& operator=(T v)    {   CommType::v = v;  return *this;   }
            };
            
            template<class T, template<class> class CT>   MPIntf& operator<<(const CT<T> &v);
            template<class T>   MPIntf& operator>>(T &v);
            
            template<class T, class... args> struct sendtask
            {
                sendtask(const args&... a) {   T::operator()(a...);   }
            };
            

            template<class T> MPIntf& operator>>(const sendtask<T> &t)
            {
                return *this;
            }
	};

}

#include "MPIntf.cpp"
