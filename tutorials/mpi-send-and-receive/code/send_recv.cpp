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
#include <cstdlib>
#include <iostream>
 
#include "MPIntf.h"
#include "PrintValue.h"

using std::cout;
using std::endl;

int main(int argc, char** argv) 
{
		// define the function call
	TaskMixInBase<MPITask::ProtoTypeForSingleVariable, int> pvInt			{	MPITask::PrintValue	};
	TaskMixInBase<MPITask::ProtoTypeForSingleVariable, float> pvFloat		{	MPITask::PrintValue	};
	TaskMixInBaseContainer<MPITask::ProtoTypeForContainer, int, std::vector> pvCont	{	MPITask::PrintValue	};

	struct MPITask::PrintValue pv;	// struct - quality to resolve shadow of PrintValue function and struct PrintValue

		// define MPI world
	using MPIDemo = MPIntf<decltype(pvFloat), decltype(pv), decltype(pvCont)>;
	MPIDemo mp { pvFloat, pv, pvCont };

	int world_size = mp.size();
	int world_rank = mp.rank();

	cout << "World size: " << world_size << endl;
	cout << "World rank: " << world_rank << endl;

	int number = 0;
	float n2 = 0.0;
	std::vector<int> sendVec { 1,3,5,7,9 }, recvVec;
	std::vector<int> applyVec { -1,-3,-5,-7,-9 };

    	if(world_rank == 0)	{	number = -1;	n2 = 7.77;	}

	mp << sendVec;
	mp >> recvVec >> MPIDemo::runcontainertask<int, std::vector>(recvVec);

	mp << applyVec;
	mp >> recvVec >> MPIDemo::Apply<MPITask::ProtoTypeForContainer, int, std::vector>(MPITask::PrintValue, recvVec);

    	mp << number << n2;
	mp >> number >> n2 >> MPIDemo::runtask<int>(number) >> MPIDemo::runtask<float>(n2);

    	mp << number << n2;
	mp >> number >> MPIDemo::runtask<int>(number) >> n2 >> MPIDemo::runtask<float>(n2);

    	return 0;
}
