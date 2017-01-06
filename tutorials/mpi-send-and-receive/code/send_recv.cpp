// Author: Wes Kendall
// Copyright 2011 www.mpitutorial.com
// This code is provided freely with the tutorials on mpitutorial.com. Feel
// free to modify it for your own use. Any distribution of the code must
// either provide a link to www.mpitutorial.com or keep this header intact.
//
// MPI_Send, MPI_Recv example. Communicates the number -1 from process 0
// to process 1.
//
#include <mpi.h>
#include <stdlib.h>
#include <iostream>
 
#include "MPIntf.h"

using std::cout;
using std::endl;

int main(int argc, char** argv) 
{
		// define the function call
	CSE856::FunctCall<CSE856::PrintValueFunctType, int> pv	{	CSE856::PrintValue	};

		// define MPI world
	using MPDemo = CSE856::MPIntf<decltype(pv)>;
	MPDemo mp { pv };

	int world_size = mp.size();
	int world_rank = mp.rank();

	cout << "World size: " << world_size << endl;
	cout << "World rank: " << world_rank << endl;

    	MPDemo::CommType<int> number;

    	if(world_rank == 0)	number = -1;
    	mp << number;
	//mp >> number;
	//if(world_rank == 1)	CSE856::PrintValue(number);
	mp >> number >> MPDemo::runtask<int>(number);

    	return 0;
}
