#include <stdio.h>

static long num_steps = 100000;
double step;
void main()
{
	int i; 
	double x,pi,sum = 0.0;

	step = 1.0 / (double) num_steps;

	for(i=0; i<num_steps; i++)
	{
		x = (i+.5)*step;
		sum += 4.0 / (1.0 + x*x);
	}
	pi = step * sum;
	printf("Pi = %f\n", pi);

}

