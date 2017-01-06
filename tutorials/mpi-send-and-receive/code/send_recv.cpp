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
	CSE856::FunctCall<CSE856::PrintValueFunctType, int> pv	{	CSE856::PrintValue	};
	using MPDemo = CSE856::MPIntf<decltype(pv)>;
	MPDemo mp { pv };

	int world_size = mp.size();
	int world_rank = mp.rank();

	cout << "World size: " << world_size << endl;
	cout << "World rank: " << world_rank << endl;

    	MPDemo::CommType<int> number;

    	number = -1;
    	mp << number;
	mp >> number;
	if(world_rank == 1)	CSE856::PrintValue(number);
	//mp >> MPDemo::sendtask< 
    	//mp >> MPDemo::setparalleltask<CSE856::PrintValueTask<int>>(CSE856::PrintValue) >>  number;

    	return 0;
}
