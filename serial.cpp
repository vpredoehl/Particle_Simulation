#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "logging.h"
#include "common.h"

#include <algorithm>

using namespace std;
//
//  benchmarking program
//



int main( int argc, char **argv )
{    
    int navg,nabsavg=0;
    double davg,dmin, absmin=1.0, absavg=0.0;

    int navg_stock,nabsavg_stock=0;
    double davg_stock,dmin_stock, absmin_stock=1.0, absavg_stock=0.0;


    if( find_option( argc, argv, "-h" ) >= 0 )
    {
        printf( "Options:\n" );
        printf( "-h to see this help\n" );
        printf( "-n <int> to set the number of particles\n" );
        printf( "-th <int> to set the number of threads\n" );
        printf( "-o <filename> to specify the output file name\n" );
        printf( "-s <filename> to specify a summary file name\n" );
        printf( "-no turns off all correctness checks and particle output\n");
        return 0;
    }
    
	extern unsigned long numThreads;
    extern short binsPerRow, binsPerCol;
    extern GhostZoneListByThread binGZList;
    extern vector<NeighborRegionList> nrl;
    extern BinGhostZoneList bgz; 

    int n = read_int( argc, argv, "-n", 1000 );
	numThreads = read_int(argc,argv,"-th",2);

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
    bgz = binGZList[numThreads];
        //    nrl =  neighborGZLayout[numThreads];
    
        // sort nrl ( neighbor region list )  if we are logging it
        //    if(LogLevel(LL::neighborgzlist))    for_each(nrl.begin(), nrl.end(), [](NeighborRegionList &l)  {   sort(l.begin(), l.end());   });

    char *savename = read_string( argc, argv, "-o", NULL );
    char *sumname = read_string( argc, argv, "-s", NULL );
    
    FILE *fsave = savename ? fopen( savename, "w" ) : NULL;
    FILE *fsum = sumname ? fopen ( sumname, "a" ) : NULL;
    
    Mesh world { numThreads };
    
    int i;
    Mesh::iterator worldIter;
        // set bin id's
    for(worldIter = world.begin(), i=0; worldIter != world.end(); worldIter++, i++)     worldIter->id = i;

		//
		// setup bin layout 
		//
    switch(numThreads)  
    {
                //      layout for two bins
                //  _________________________
                //  |           |           |
                //  |           |           |
                //  |     0     |     1     |
                //  |           |           |
                //  |___________|___________|

        case 2:
                // if there is no adjacent bin ( i.e. because it's a wall,
                // set to -1 so there is no attempt to set a ghost zone
                // ( because there is no storage for it )
            world[0].binToLeft = world[0].binToTop = world[0].binToBottom 
                = world[0].binToUpperLeft = world[0].binToUpperRight 
                = world[0].binToLowerLeft = world[0].binToLowerRight = -1;
            world[0].binToRight = 1;
            
            world[1].binToRight = world[1].binToTop = world[1].binToBottom
                = world[1].binToUpperLeft = world[1].binToUpperRight 
                = world[1].binToLowerLeft = world[1].binToLowerRight = -1;
            world[1].binToLeft = 0;
            break;

                //      layout for four bins
                //  _________________________
                //  |           |           |
                //  |           |           |
                //  |     0     |     1     |
                //  |           |           |
                //  |___________|___________|
                //  |           |           |
                //  |           |           |
                //  |     2     |     3     |
                //  |           |           |
                //  |___________|___________|

        case 4:
			world[0].binToLeft = world[0].binToTop = world[1].binToTop 
				= world[1].binToRight = world[2].binToLeft = world[2].binToBottom
				= world[3].binToBottom = world[3].binToRight = -1;
			world[1].binToLeft = world[2].binToTop = 0;
			world[0].binToRight = world[3].binToTop = 1;
			world[0].binToBottom = world[3].binToLeft = 2;
			world[1].binToBottom = world[2].binToRight = 3;

            world[0].binToUpperLeft = world[0].binToUpperRight = world[0].binToLowerLeft = -1;
            world[0].binToLowerRight = 3;

            world[1].binToUpperLeft = world[1].binToUpperRight = world[1].binToLowerRight = -1;
            world[1].binToLowerLeft = 2;

            world[2].binToUpperLeft = world[2].binToLowerLeft = world[2].binToLowerRight = -1;
            world[2].binToUpperRight = 1;

            world[3].binToUpperRight = world[3].binToLowerLeft = world[3].binToLowerRight = -1;
            world[3].binToUpperLeft = 0;
			break;
            
                //      layout for eight bins
                //  _________________________________________________
                //  |           |           |           |           |
                //  |           |           |           |           |
                //  |     0     |     1     |     2     |     3     |
                //  |           |           |           |           |
                //  |___________|___________|___________|___________|
                //  |           |           |           |           |
                //  |           |           |           |           |
                //  |     4     |     5     |     6     |     7     |
                //  |           |           |           |           |
                //  |___________|___________|___________|___________|
        case 8:
                // set perimeter walls
            world[0].binToLeft = world[0].binToTop = world[1].binToTop = world[2].binToTop
                = world[3].binToTop = world[3].binToRight = world[4].binToLeft
                = world[4].binToBottom = world[5].binToBottom = world[6].binToBottom
                = world[7].binToBottom = world[7].binToRight = -1;
            
            world[0].binToUpperLeft = world[0].binToUpperRight = world[0].binToLowerLeft = -1;
            world[0].binToRight = 1;
            world[0].binToBottom = 4;
            world[0].binToLowerRight = 5;
            
            world[1].binToUpperLeft = world[1].binToUpperRight = -1;
            world[1].binToLeft = 0;
            world[1].binToRight = 2;
            world[1].binToLowerLeft = 4;
            world[1].binToBottom = 5;
            world[1].binToLowerRight = 6;
            
            world[2].binToUpperLeft = world[2].binToUpperRight = -1;
            world[2].binToLeft = 1;
            world[2].binToRight = 3;
            world[2].binToLowerLeft = 5;
            world[2].binToBottom = 6;
            world[2].binToLowerRight = 7;
            
            world[3].binToUpperLeft = world[3].binToUpperRight = world[3].binToLowerRight -1;
            world[3].binToLeft = 2;
            world[3].binToLowerLeft = 6;
            world[3].binToBottom = 7;
            
            world[4].binToUpperLeft = world[4].binToLowerLeft = world[4].binToLowerRight = -1;
            world[4].binToTop = 0;
            world[4].binToUpperRight = 1;
            world[4].binToRight = 5;
            
            world[5].binToLowerLeft = world[5].binToLowerRight = -1;
            world[5].binToLeft = 4;
            world[5].binToUpperLeft = 0;
            world[5].binToTop = 1;
            world[5].binToUpperRight = 2;
            world[5].binToRight = 6;
            
            world[6].binToLowerLeft = world[6].binToLowerRight = -1;
            world[6].binToLeft = 5;
            world[6].binToUpperLeft = 1;
            world[6].binToTop = 2;
            world[6].binToUpperRight = 3;
            world[6].binToRight = 7;
            
            world[7].binToUpperRight = world[7].binToLowerLeft = world[7].binToLowerRight = -1;
            world[7].binToLeft = 6;
            world[7].binToUpperLeft = 2;
            world[7].binToTop = 3;
            break;
    }

    particle_t *particles = (particle_t*) malloc( n * sizeof(particle_t) );
    set_size( n );
    init_particles( n, world, particles );

    nrl = NeighborGZListFromBinGZList(world);  // depends on bin id which is initialized in init_particles ( above )
    if(LogLevel(LL::neighborgzlist))        cout << nrl << endl;


        // run serial test side by side to compare results with multi-bin run
    SerialRunTest srt{world};
    
    if( fsum) 
    {
        short numP = 0;
        
        for_each(world.begin(), world.end(),
            [fsum, &numP](const Bin &b)    {    short s = list_size(b.content); fprintf(fsum,"Bin Size: %i  Crossovers: %li\n",s, b.crossovers.size()); numP += s;  });

        fprintf(fsum, "Initial total particles in bins: %i\n", numP);
    }

    
    //
    //  simulate a number of time steps
    //
    double simulation_time = read_timer( );
    auto stockIter = [=,&dmin_stock, &davg_stock, &navg_stock](int step)
        {
            //
            //  compute forces
            //
            for( int i = 0; i < n; i++ )
            {
                particles[i].ax = particles[i].ay = 0;
                for (int j = 0; j < n; j++ )
//                for (int j = i+1; j < n; j++ )
                    apply_force( particles[i], particles[j],&dmin_stock,&davg_stock,&navg_stock);
            }
     
            //
            //  move particles
            //
            for( int i = 0; i < n; i++ ) 
                move( particles[i] );		
        };
    
	
 for( int step = 0; step < NSTEPS; step++ )
  {
        if(LogLevel(LL::content))
        {
            short s = list_size(srt.srtWorld);
            if(particles)
            {
                cout << "Stock particles: " << endl;
                for(int i = 0; i<s; i++) cout << particles[i] << endl;
            }
        }
        if(LogLevel(LL::step)) cout << "Begin step: " << step << endl;
  
        for_each(world.begin(), world.end(), 
            [step](Bin &b)
        {
            if(LogLevel(LL::content))   
			{
				cout << "Begin: Step " << step << ": bin id: " << b.id << endl << b << endl;            
            	for(auto contentIter = b.content.cbegin(); contentIter != b.content.cend(); )
            	{
                	auto v = *contentIter;
                	if(find(++contentIter,b.content.cend(), v) != b.content.cend())
                    	cout << "Step: " << step << " unique content test failed: " << v << endl;
            	}
			}

            
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
                if(LogLevel(LL::crossover)) cout << "Absorbing Crossover: Step " << step << "  " << p << endl;
            });
            if(LogLevel(LL::crossover))   cout << "Step: " << step << "  Crossovers: " << b.crossovers.size() << "  BinSize: " << list_size(b.content) << endl;
            b.crossovers.clear();   // erase contents of crossover list so they won't be absorbed again
        });
			// BARRIER: absorb crossovers
        
	navg = 0;
        davg = 0.0;
	dmin = 1.0;
        
        //
        //  compute forces
        // 
        for_each(world.begin(), world.end(), 
        [&](Bin &b)
        {          
            if(LogLevel(LL::serialruntest))	b.content.sort();
            auto particleIter = b.content.begin();

            cout << "Mesh Compute Forces: Step " << step << endl;
            if(LogLevel(LL::content))    cout << ": bin id: " << b.id << endl << b << endl;
            
            while(particleIter != b.content.end())
            {
                auto& p1 = *particleIter;                
                auto Interact = [&](const particle_t &p2)   {   apply_force( p1, p2 ,&dmin,&davg,&navg);    };
                forward_list<particle_t>::const_iterator neighborIter = particleIter;

                p1.ax = p1.ay = 0;
                for_each(b.content.cbegin(), b.content.cend(),  Interact);
                        
                        //
                        // apply force in caused by neighboring ghost zones
                        //
                    if(b.binToLeft != -1)
                    {
                            // get right ghost zone for bin to left and apply force
                        vector<particle_t> &rightGZ = world[b.binToLeft].gz[static_cast<int>(GZR::right)];
                        for_each(rightGZ.begin(), rightGZ.end(), Interact);
                    }         
                    if(b.binToRight != -1)
                    {
                            // get left ghost zone for bin to right and apply force
                        vector<particle_t> &leftGZ = world[b.binToRight].gz[static_cast<int>(GZR::left)];
                        for_each(leftGZ.begin(), leftGZ.end(), Interact);
                    }
                    if(b.binToTop != -1)
                    {
                        vector<particle_t> &bottomGZ = world[b.binToTop].gz[static_cast<int>(GZR::bottom)];
                        for_each(bottomGZ.begin(), bottomGZ.end(), Interact);
                    }
                    if(b.binToBottom != -1)
                    {
                        vector<particle_t> &topGZ = world[b.binToBottom].gz[static_cast<int>(GZR::top)];
                        for_each(topGZ.begin(), topGZ.end(), Interact);
                    }
                    if(b.binToUpperLeft != -1)
                    {
                        vector<particle_t> lowerRightGZ = world[b.binToUpperLeft].bottomRightGZ();
                        for_each(lowerRightGZ.begin(), lowerRightGZ.end(), Interact);
                    }
                    if(b.binToUpperRight != -1)
                    {
                        vector<particle_t> lowerLeftGZ = world[b.binToUpperRight].bottomLeftGZ();
                        for_each(lowerLeftGZ.begin(), lowerLeftGZ.end(), Interact);
                    }
                    if(b.binToLowerLeft != -1)
                    {
                        vector<particle_t> upperRightGZ = world[b.binToLowerLeft].topRightGZ();
                        for_each(upperRightGZ.begin(), upperRightGZ.end(), Interact);
                    }
                    if(b.binToLowerRight != -1)
                    {
                        vector<particle_t> upperLeftGZ = world[b.binToLowerRight].topLeftGZ();
                        for_each(upperLeftGZ.begin(), upperLeftGZ.end(), Interact);
                    }

                particleIter++;
            }
        });
			// BARRIER:  compute forces

            // run serial run test for apply_force after each bin
            // has completed its update
        if(LogLevel(LL::serialruntest))
        {
            if(LogLevel(LL::interaction))   cout << "srt: Compute Forces: Step " << step << endl;
            srt.interact(apply_force);
                // and compare
            if(LogLevel(LL::interaction) && srt != world)    cout << "srt apply_force diverged: step " << step << endl;
            if(LogLevel(LL::interaction) && srt != particles)    cout << "stock code apply_force diverged: step " << step << endl;
        }
 
        //
        //  move particles
        //
        for_each(world.begin(), world.end(),  
            [&](Bin &b)
            {        
                if(LogLevel(LL::serialruntest))	b.content.sort();
                auto  particleIter = b.content.begin(), lastIter = particleIter;    // lastIter is for erase_after so current particle can be deleted
                
                    // clear all ghost zone regions so they can be updated
                    // after particles move
                for_each(b.gz.begin(), b.gz.end(), [](vector<particle_t> &c)    {   c.clear();  });

                if(LogLevel(LL::content))    cout << "Mesh Move: Step " << step << ": bin id: " << b.id << endl << b << endl;


                    // iterate through particles in content container
                while(particleIter != b.content.end())
                {        
                    auto& p = *particleIter;
                    move(p);

                    bool jumpLeft = p.x < b.leftWall;
                    bool jumpRight = p.x > b.rightWall;
                    bool jumpTop = p.y < b.topWall;
                    bool jumpBottom = p.y > b.bottomWall;
            
                        // add the particle to the neighboring bin if there  is a crossover
                    if(jumpLeft)
                    {
                        if(jumpTop && b.binToUpperLeft != -1)           world[b.binToUpperLeft].crossovers.push_back(p);
                        else if(jumpBottom && b.binToLowerLeft != -1)   world[b.binToLowerLeft].crossovers.push_back(p);
                        else if(b.binToLeft != -1) world[b.binToLeft].crossovers.push_back(p);
                            // world[ jumpTop ? b.binToUpperLeft : jumpBottom ? b.binToLowerLeft : b.binToLeft ].crossovers.push_back(p);
                    }
                    else if(jumpRight)
                    {
                        if(jumpTop && b.binToUpperRight != -1)  world[b.binToUpperRight].crossovers.push_back(p);
                        else if(jumpBottom && b.binToLowerRight != -1)   world[b.binToLowerRight].crossovers.push_back(p);
                        else if(b.binToRight != -1) world[b.binToRight].crossovers.push_back(p);
                            // world[ jumpTop ? b.binToUpperRight : jumpBottom ? b.binToLowerRight : b.binToRight ].crossovers.push_back(p);
                    }
                    else if(jumpTop)    world[ b.binToTop ].crossovers.push_back(p);
                    else if(jumpBottom) world[ b.binToBottom ].crossovers.push_back(p);     
            
                    if(jumpLeft || jumpRight || jumpTop || jumpBottom)
                    {
                            // particle left jumped - delete it from this bin
                        if(LogLevel(LL::crossover)) cout << "Crossover: " << p << endl;
                        if(lastIter == particleIter)
                        {
                            b.content.pop_front();  // happened to be the first in the list
                            lastIter = particleIter = b.content.begin();    // reset iterators
                        }
                        else 
                        {
                            particleIter = b.content.erase_after(lastIter); // IMPLEMENTATION BUG:  Doesn't return element after one erased.  Returns first element in list instead
                            particleIter = std::next(lastIter);		// workaround
                        }
                    }
                    else
                    {
                            // ghost zone maintentance
                        bool inLeftGZ = p.x < b.leftWall + cutoff;
                        bool inRightGZ = p.x > b.rightWall - cutoff;
                        bool inTopGZ = p.y < b.topWall + cutoff;
                        bool inBottomGZ = p.y > b.bottomWall - cutoff;
                        
                            // check each enumerated ghost zone in the list ( gzl ) and add
                            // particle if it is in ghost zone
							// particles in corner gz's, e.g. upper left will be 
							// added to both top and left gz's, for example
                        for_each(b.gzl.cbegin(), b.gzl.cend(),
                            [&,inLeftGZ,inRightGZ, inTopGZ, inBottomGZ](GhostZoneRegion r)
                            {
                                vector<particle_t>::const_iterator gzIter;
                                
                                if(LogLevel(LL::gz) && r < GZR::num_walls)    gzIter = find_if(b.gz[static_cast<int>(r)].cbegin(), b.gz[static_cast<int>(r)].cend(),
                                    [p](particle_t v) -> bool    {   return p.id == v.id;   });
                               
                                switch(r)
                                {
                                    case GZR::left:     
                                        if(inLeftGZ)    
                                        {
                                            if(LogLevel(LL::gz) && r < GZR::num_walls && gzIter != b.gz[static_cast<int>(r)].cend())
                                                cout << "pushing duplicate gz L  " << p << endl;
                                            b.gz[static_cast<int>(r)].push_back(p);
                                        }
                                        break;
                                    case GZR::right:    
                                        if(inRightGZ)
                                        {
                                            if(LogLevel(LL::gz) && r < GZR::num_walls && gzIter != b.gz[static_cast<int>(r)].cend())
                                                cout << "pushing duplicate gz R  " << p << endl;
                                            b.gz[static_cast<int>(r)].push_back(p);
                                        }
                                        break;
                                    case GZR::top:      
                                        if(inTopGZ)
                                        {
                                            if(LogLevel(LL::gz) && r < GZR::num_walls && gzIter != b.gz[static_cast<int>(r)].cend())  cout << "pushing duplicate gz T  " << p << endl;
                                            b.gz[static_cast<int>(r)].push_back(p);
                                        }
                                        break;                                    
                                    case GZR::bottom:   
                                        if(inBottomGZ)  
                                        {
                                            if(LogLevel(LL::gz) && r < GZR::num_walls && gzIter != b.gz[static_cast<int>(r)].cend())  cout << "pushing duplicate gz B  " << p << endl;
                                            b.gz[static_cast<int>(r)].push_back(p);
                                        }
                                        break;
                                }
                            });
                    
                        lastIter = particleIter++;  // go to the next particle
                    }
                }   // while

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
			// BARRIER:  move

        if(LogLevel(LL::serialruntest))
        {
            srt.move(::move);
                // and compare
            if(LogLevel(LL::content) && LogLevel(LL::move))
            {
                cout << "srt Move: " << endl;
                cout << "Mesh contents: " << world << endl;
                cout << "srt move contents: " << endl << srt.srtWorld << endl;
            }
            if(particles)   stockIter(step);    // run actual stock code
            if(srt != particles)    cout << "stock code move diverged: step " << step << endl;
            if(srt != world)    cout << "srt move diverged: step " << step << endl;
            
        }
  } // for
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
            [fsum, &numP](const Bin &b)    {    short s = list_size(b.content); fprintf(fsum,"Bin Size: %i  Crossovers: %li\n",s, b.crossovers.size()); numP += s;  });
        
        fprintf(fsum, "Total particles in bins: %i\n", numP);
    }
    
    for_each(world.begin(), world.end(),
             [](const Bin &b)
             {
                 for_each(b.content.begin(), b.content.end(),
                          [](const particle_t &p) {   cout << "v:  ( " << p.vx << ", " << p.vy << " ) " << endl;  });
             });
    if(LogLevel(LL::serialruntest))
    {
        if(srt != particles)    cout << "stock code diverged"  << endl;    else    cout << "stock code matches!!" << endl;
        if(srt != world)    cout << "srt diverged: step " << endl; else cout << "srt code matches!!" << endl;
        if(world != particles)  cout << "Mesh diverged from stock code" << endl;    else    cout << "Mesh matches stock code!!" << endl;
    }

    //
    // Clearing space
    //
    if( fsum )
        fclose( fsum );    
    if( fsave )
        fclose( fsave );
    if(particles)   free(particles);        

    
    return 0;
}
