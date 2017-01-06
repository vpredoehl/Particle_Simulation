namespace CSE856
{
	template<class PROTOTYPE, class... ARGS>
	class FunctCall
	{
		PROTOTYPE fn;

		public:
			FunctCall(PROTOTYPE f) : fn{f}  {}

			virtual void operator()(const ARGS&... args)	{	}
	};

 	template<class T>
        using PrintValueTask = void(*)(T);

	void PrintValue(int v);
}

