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

	};

}
