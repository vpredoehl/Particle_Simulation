#include <stdio.h>

static long num_steps = 100000;
double step;
void main()
{
	int i; 
	double x,pi,pi2,sum = 0.0;
	double sumV[4] = { 0.0, 0.0, 0.0, 0.0 } ;

	omp_set_num_threads(4);
	step = 1.0 / (double) num_steps;

#pragma omp parallel for reduction(+:sum), private(x,i)   /* i private by default? */
	for(i=0; i<num_steps; i++)
	{
		int id = omp_get_thread_num();

		x = (i+.5)*step;

		sum += 4.0 / (1.0 + x*x);
		sumV[id] += 4.0 / (1.0 + x*x);
	}
	pi = step * sum;
	pi2 = ( sumV[0] + sumV[1] + sumV[2] + sumV[3] ) * step;

	printf("Pi = %f\n", pi);
	printf("Pi2 = %f\n", pi2);
}

