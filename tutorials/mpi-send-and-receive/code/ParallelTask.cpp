#include "ParallelTask.h"

#include <iostream>

void CSE856::PrintValue(int v)
{
    std::cout << "Process 1 received number " << v << " from process 0\n";
}

