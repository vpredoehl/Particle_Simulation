namespace CSE856
{
	class MPIntf
	{
        	int world_rank, world_size;

	public:
		MPIntf();
		~MPIntf();

			// accessor methods
		int rank() const	{	return world_rank;	}
		int size() const	{	return world_size;	}

                // wrapper class for communication types
            template<class T>
            class CommType
            {
                T v;
            public:
            
                T value() const {   return v;   }
                operator T() const  {   return v;   }
                const T* operator&() const    {   return &v;  }
                T* operator&()    {   return &v;  }
                
                CommType& operator=(T v)    {   CommType::v = v;  return *this;   }
            };
            
            template<class T>   MPIntf& operator<<(const CommType<T> &v);
            template<class T>   MPIntf& operator>>(CommType<T> &v);
            

        private:
            template<class T>
            using ParallelTask = void(*)(T);

            ParallelTask<int> parallelTask;
                // stream modifier to set parallel task
                
        public:
            
            template<class T> class setparalleltask
            {
                ParallelTask<T> task;
            public:
                setparalleltask(ParallelTask<T> t) {   task = t;   }
                operator ParallelTask<T>() const {   return task;    }
            };
            

            template<class T> MPIntf& operator>>(const setparalleltask<T> &t)
            {
                parallelTask = t;
                return *this;
            }
	};

}


// define template operators here to remove clutter from interface
template<class T>
CSE856::MPIntf& CSE856::MPIntf::operator<<(const CSE856::MPIntf::CommType<T> &v)
{
    if(world_rank == 0)    MPI_Send(&v, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    return *this;
}

template<class T>
CSE856::MPIntf& CSE856::MPIntf::operator>>(CSE856::MPIntf::CommType<T> &v)
{
    if(world_rank != 0)
    {
        MPI_Recv(&v, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
            // run task 
        if(parallelTask)
            (*parallelTask)(v);
    }
    return *this;
}

