namespace CSE856
{
	class MPIntf
	{
//                template<class T> class CommType;
                typedef  void (*ParallelTask)(int);
	
        	int world_rank, world_size;
                ParallelTask parallelTask;

	public:
		MPIntf();
		~MPIntf();

			// accessor methods
		int rank() const	{	return world_rank;	}
		int size() const	{	return world_size;	}

                // stream modifier to set parallel task
            class setparalleltask
            {
                ParallelTask task;
            public:
                setparalleltask(ParallelTask t) {   task = t;   }
                operator ParallelTask() const {   return task;    }
            };
            
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
            MPIntf& operator>>(setparalleltask &t)
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

