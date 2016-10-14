#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "common.h"

#include <iostream>
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
    extern vector<NeighborRegionList> nr;

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

    switch(numThreads)  // set adjacent bin indexes 
    {
        case 2:
            world[0].binToLeft = world[0].binToTop = world[0].binToBottom 
                = world[0].binToUpperLeft = world[0].binToUpperRight 
                = world[0].binToLowerLeft = world[0].binToLowerRight = -1;
            world[0].binToRight = 1;
            
            world[1].binToRight = world[1].binToTop = world[1].binToBottom
                = world[1].binToUpperLeft = world[1].binToUpperRight 
                = world[1].binToLowerLeft = world[1].binToLowerRight = -1;
            world[1].binToLeft = 0;
            break;
    }    

    set_size( n );
    init_particles( n, world );
    
    if( fsum) 
    {
        short numP = 0;
        
        for_each(world.begin(), world.end(),
            [fsum, &numP](const Bin &b)    {    short s = list_size(b.content); fprintf(fsum,"Bin Size: %i  Crossovers: %i\n",s, b.crossovers.size()); numP += s;  });

        fprintf(fsum, "Initial total particles in bins: %i\n", numP);
    }

    
    //
    //  simulate a number of time steps
    //
    double simulation_time = read_timer( );
	
 for( int step = 0; step < NSTEPS; step++ )
  {
        for_each(world.begin(), world.end(), 
            [step](Bin &b)
        {
           // 
            // absorb crossovers from other bins first
            //
            for_each(b.crossovers.cbegin(), b.crossovers.cend(), 
          [step,&b](const particle_t &p)
          {
                 // place new particle in appropriate ghost zone for this bin
                 // so interaction in neighboring bin is not affected
             if(p.x < b.leftWall + cutoff) b.gz[static_cast<int>(GZR::left)].push_back(p);
             else if(p.x > b.rightWall - cutoff) b.gz[static_cast<int>(GZR::right)].push_back(p);
             if(p.y < b.topWall + cutoff) b.gz[static_cast<int>(GZR::top)].push_back(p);
             else if(p.y > b.bottomWall - cutoff) b.gz[static_cast<int>(GZR::bottom)].push_back(p);
          
                  // then add new particle to this bin's content list
             b.content.push_front(p);    
           });
            cout << "Step: " << step << "  Crossovers: " << b.crossovers.size() << "  BinSize: " << list_size(b.content) << endl;
            b.crossovers.clear();   // erase contents of crossover list so they won't be absorbed again
        });
        
	navg = 0;
        davg = 0.0;
	dmin = 1.0;
        
        //
        //  compute forces
        // 
        for_each(world.begin(), world.end(), 
        [&](Bin &b)
        {          
                for_each(b.content.begin(), b.content.end(),
                [&,b](particle_t &p1)
                {   
                    auto ApplyForce = [&](const particle_t &p2)   {   apply_force( p1, p2 ,&dmin,&davg,&navg);    };

                    p1.ax = p1.ay = 0;
                    for_each(b.content.begin(), b.content.end(),  ApplyForce);
                        
                        //
                        // apply force in caused by neighboring ghost zones
                        //
                    if(b.binToLeft != -1)
                    {
                            // get right ghost zone for bin to left and apply force
                        vector<particle_t> &rightGZ = world[b.binToLeft].gz[static_cast<int>(GZR::right)];
                        for_each(rightGZ.begin(), rightGZ.end(), ApplyForce);
                    }         
                    if(b.binToRight != -1)
                    {
                            // get left ghost zone for bin to right and apply force
                        vector<particle_t> &leftGZ = world[b.binToRight].gz[static_cast<int>(GZR::left)];
                        for_each(leftGZ.begin(), leftGZ.end(), ApplyForce);
                    }
                    if(b.binToTop != -1)
                    {
                        vector<particle_t> &bottomGZ = world[b.binToTop].gz[static_cast<int>(GZR::bottom)];
                        for_each(bottomGZ.begin(), bottomGZ.end(), ApplyForce);
                    }
                    if(b.binToBottom != -1)
                    {
                        vector<particle_t> &topGZ = world[b.binToBottom].gz[static_cast<int>(GZR::top)];
                        for_each(topGZ.begin(), topGZ.end(), ApplyForce);
                    }
//                    if(p1.ax > .1 || p1.ay > .1)    cout << "step: " << step << " a: ( " << p1.ax << ", " << p1.ay << " )" << endl;
                    
                }); 
        });

 
        //
        //  move particles
        //
        for_each(world.begin(), world.end(), 
            [&](Bin &b)
            {        
                auto  particleIter = b.content.begin(), lastIter = particleIter;    // lastIter is for erase_after so current particle can be deleted
                while(particleIter != b.content.end())
                {        
                    auto& p = *particleIter;
                    move(p);

            bool jumpLeft = p.x < b.leftWall;
            bool jumpRight = p.x > b.rightWall;
            bool jumpTop = p.y < b.topWall;
            bool jumpBottom = p.y > b.bottomWall;
            
                // add the particle to the neighboring bin if there  is a crossover
//            if(jumpLeft)        world[ jumpTop ? b.binToUpperLeft : jumpBottom ? b.binToLowerLeft : b.binToLeft ].crossovers.push_back(p);
  //          else if(jumpRight)  world[ jumpTop ? b.binToUpperRight : jumpBottom ? b.binToLowerRight : b.binToRight ].crossovers.push_back(p);
            if(jumpLeft)        world[ b.binToLeft ].crossovers.push_back(p);
            else if(jumpRight)  world[ b.binToRight ].crossovers.push_back(p);
            if(jumpTop)    world[ b.binToTop ].crossovers.push_back(p);
            else if(jumpBottom) world[ b.binToBottom ].crossovers.push_back(p);     
            
            if(jumpLeft || jumpRight || jumpTop || jumpBottom)
            {
                    // particle left jumped - delete it from this bin
//                cout << "step: " << step << " BinSize: " << list_size(b.content) << "  Crossover ( " << p.x << ", " << p.y << " ), ( vx: " << p.vx << ", " << " vy: " << p.vy << " )  jumpLeft: " << jumpLeft << " jumpRight: " << jumpRight << " jumpTop: " << jumpTop << " jumpBottom: " << jumpBottom << endl;
                if(lastIter == particleIter)
                {
                    b.content.pop_front();  // happened to be the first in the list
                    lastIter = particleIter = b.content.begin();    // reset iterators
                }
                else particleIter = b.content.erase_after(lastIter);
                continue;
            }
            lastIter = particleIter++;
        };

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
    });
  }
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
            [fsum, &numP](const Bin &b)    {    short s = list_size(b.content); fprintf(fsum,"Bin Size: %i  Crossovers: %i\n",s, b.crossovers.size()); numP += s;  });

        for_each(world.begin(), world.end(),
            [](const Bin &b)
            {
                for_each(b.content.begin(), b.content.end(),
                    [](const particle_t &p) {   cout << "v:  ( " << p.vx << ", " << p.vy << " ) " << endl;  });
            });
            
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
