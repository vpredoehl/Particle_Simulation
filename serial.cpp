#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "common.h"

#include <algorithm>

using namespace std;
//
//  benchmarking program
//
template<class T>
inline short list_size(const forward_list<T> &l)   {   short cnt = 0;  auto b = l.begin(); while(b != l.end())  {   b++;    cnt++;  }  return cnt; }

int main( int argc, char **argv )
{    
    int navg,nabsavg=0;
    double davg,dmin, absmin=1.0, absavg=0.0;

    if( find_option( argc, argv, "-h" ) >= 0 )
    {
        printf( "Options:\n" );
        printf( "-h to see this help\n" );
        printf( "-n <int> to set the number of particles\n" );
        printf( "-o <filename> to specify the output file name\n" );
        printf( "-s <filename> to specify a summary file name\n" );
        printf( "-no turns off all correctness checks and particle output\n");
        return 0;
    }
    
    int n = read_int( argc, argv, "-n", 1000 );
    extern short binsPerRow, binsPerCol, numThreads;
    extern BinNeighbor neighborBin;
    extern vector<NeighborRegion> nr;

        // determine mesh size
    switch(numThreads)
    {
        case 16: binsPerRow = 4; binsPerCol = 4; break;
        case 8: binsPerRow = 4; binsPerCol = 2; break;
        case 4: binsPerRow = 2; binsPerCol = 2; break;
        case 2: binsPerRow = 2; binsPerCol = 1; break;
        case 1:            
            // fall thru
        default:
            binsPerRow = binsPerCol = 1;
    }
    nr =  neighborBin[numThreads];

    char *savename = read_string( argc, argv, "-o", NULL );
    char *sumname = read_string( argc, argv, "-s", NULL );
    
    FILE *fsave = savename ? fopen( savename, "w" ) : NULL;
    FILE *fsum = sumname ? fopen ( sumname, "a" ) : NULL;
    
    Mesh world { numThreads, Bin{} };
    set_size( n );
    init_particles( n, world );
    
    //
    //  simulate a number of time steps
    //
    double simulation_time = read_timer( );
	
  for_each(world.begin(), world.end(), 
  [&](Bin &b)
  {  
    for( int step = 0; step < NSTEPS; step++ )
    {
	navg = 0;
        davg = 0.0;
	dmin = 1.0;
        //
        //  compute forces
        // 
        
                for_each(b.content.begin(), b.content.end(), 
                [&,b](particle_t &p1)
                {   
                    p1.ax = p1.ay = 0;
                    for_each(b.content.begin(), b.content.end(), 
                    [&](const particle_t &p2)
                    {
                        apply_force( p1, p2 ,&dmin,&davg,&navg);
                    });
                    
                }); 

 
        //
        //  move particles
        //
        for_each(b.content.begin(), b.content.end(), [](particle_t &p)   {   move(p);    });

        if( find_option( argc, argv, "-no" ) == -1 )
        {
          //
          // Computing statistical data
          //
          if (navg) {
            absavg +=  davg/navg;
            nabsavg++;
          }
          if (dmin < absmin) absmin = dmin;
		
          //
          //  save if necessary
          //
          if( fsave && (step%SAVEFREQ) == 0 )
              save( fsave, n, world );
        }
    }
  });
    simulation_time = read_timer( ) - simulation_time;
    
    printf( "n = %d, simulation time = %g seconds", n, simulation_time);

    if( find_option( argc, argv, "-no" ) == -1 )
    {
      if (nabsavg) absavg /= nabsavg;
    // 
    //  -the minimum distance absmin between 2 particles during the run of the simulation
    //  -A Correct simulation will have particles stay at greater than 0.4 (of cutoff) with typical values between .7-.8
    //  -A simulation were particles don't interact correctly will be less than 0.4 (of cutoff) with typical values between .01-.05
    //
    //  -The average distance absavg is ~.95 when most particles are interacting correctly and ~.66 when no particles are interacting
    //
    printf( ", absmin = %lf, absavg = %lf", absmin, absavg);
    if (absmin < 0.4) printf ("\nThe minimum distance is below 0.4 meaning that some particle is not interacting");
    if (absavg < 0.8) printf ("\nThe average distance is below 0.8 meaning that most particles are not interacting");
    }
    printf("\n");     

    //
    // Printing summary data
    //
    if( fsum) 
    {
        short numP = 0;
        
        fprintf(fsum,"%d %g\n",n,simulation_time);
        for_each(world.begin(), world.end(),
            [fsum, &numP](const Bin &b)    {    short s = list_size(b.content); fprintf(fsum,"Bin Size: %i\n",s); numP += s;  });

        fprintf(fsum, "Total particles in bins: %i\n", numP);
    }
    //
    // Clearing space
    //
    if( fsum )
        fclose( fsum );    
    if( fsave )
        fclose( fsave );
    
    return 0;
}
