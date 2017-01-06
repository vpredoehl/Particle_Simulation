#include "ParallelTask.h"

#include <iostream>

void CSE856::PrintValue(int v)
{
    std::cout << "Process 1 received number " << v << " from process 0\n";
}

void CSE856::PrintValue(float v)
{
    std::cout << "Process 1 received number " << v << " from process 0\n";
}
