#include "logging.h"


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "common.h"

#include <algorithm>
#include <iostream>


using namespace std;

double size;
short numThreads = 2;
short binsPerRow, binsPerCol;

short particle_t::cnt = 0;


BinNeighbor neighborBin
  {
    { 2,        // ghost zones for two bins
                // the following vector is enumerated in bin order
      {
        {   // neighbor regions for bin 0
            {1, GhostZoneRegion::left}    
        },
        {   // neighbor regions for bin 1
            {0, GZR::right}
        }
      }
    },
    { 4,        // ghost zones for four bins
                // the following vector is enumerated in bin order
      {    
        {   // neighbor regions for bin 0
            {1, GZR::left}, {2, GZR::top}, {3,GZR::topLeft}
        },
        {   // neighbor regions for bin 1
            {0, GZR::right}, {2,GZR::topRight}, {3, GZR::top}
        },
        {   // neighbor regions for bin 2
            {0, GZR::bottom}, {1, GZR::bottomLeft}, {3, GZR::left}
        },
        {   // neighbor regions for bin 3
            {0, GZR::bottomRight}, {1, GZR::bottom}, {2, GZR::right}
        }
      }
    }
  };

BinList binGZ   // enumerates each bin's ghost zone regions
    {
        { 2,    // two threads
            {
                {GZR::right}, // bin 0
                {GZR::left} // bin 1
            }
        },
        { 4,    // four threads
            {
                    // ghost zone regions by bin
                {GZR::right, GZR::bottom, GZR::bottomRight},  // bin 0
                {GZR::left, GZR::bottom, GZR::bottomLeft},   // bin 1
                {GZR::right, GZR::top, GZR::topRight},     // bin 2
                {GZR::left, GZR::top, GZR::topLeft}       // bin 3
            }
        }
    };
    

// specific enumeration for current number of threads
vector<NeighborRegionList> nr;
BinGhostZoneList bgz; 

//
//  timer
//
double read_timer( )
{
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

//
//  keep density constant
//
void set_size( int n )
{
    size = sqrt( density * n );
}

//
//  Initialize the particle positions and velocities
//
void init_particles( int n, Mesh &p)
{
    auto tt = time ( NULL );
    srand48( 1476662249 );
    cout << "Seed: " << tt << endl;
    
    int sx = (int)ceil(sqrt((double)n));
    int sy = (n+sx-1)/sx;
    
    int *shuffle = (int*)malloc( n * sizeof(int) );
    for( int i = 0; i < n; i++ )
        shuffle[i] = i;
        
    
    for( int i = 0; i < n; i++ ) 
    {
        particle_t newParticle;
        
        //
        //  make sure particles are not spatially sorted
        //
        int j = lrand48()%(n-i);
        int k = shuffle[j];
        shuffle[j] = shuffle[n-i-1];
        
        //
        //  distribute particles evenly to ensure proper spacing
        //
        int x = (newParticle.x = size*(1.+(k%sx))/(1+sx)) * binsPerRow / size;
        int y = (newParticle.y = size*(1.+(k/sx))/(1+sy)) * binsPerCol / size;

        short whichBin = x + y * binsPerRow;
        Bin &dropBin = p[whichBin];

        float leftWall = dropBin.leftWall = x * size / binsPerRow;
        float rightWall = dropBin.rightWall = (x+1) * size / binsPerRow;
        float topWall = dropBin.topWall = y * size / binsPerCol;
        float bottomWall = dropBin.bottomWall = (y+1) * size/binsPerCol;
        
            // ghost zone placement
        for_each(nr.begin(), nr.end(), [=, &dropBin](const NeighborRegionList &neighbors)
            {
                    // ghoxt zone regions are optimized to find gz's for neighboring bin
                    // so we have to sequentially search the neighboring bins for gz's
                    // for this bin
                auto rgnIter = find_if(neighbors.begin(), neighbors.end(), 
                    [=, &dropBin](const pair<short /*  neighbor bin */, GhostZoneRegion> &n) -> bool    {   return n.first == whichBin; });
                if(rgnIter != neighbors.end())  
                {
                    bool inGZ = false;
                        // found a ghost zone for this bin
                        // see if current particle is in it
                    switch(rgnIter->second)
                    {
                        case GZR::left:     inGZ = newParticle.x < leftWall + cutoff;    break;
                        case GZR::top:      inGZ = newParticle.y < topWall + cutoff;     break;
                        case GZR::right:    inGZ = newParticle.x > rightWall - cutoff;   break;
                        case GZR::bottom:   inGZ = newParticle.y > bottomWall - cutoff;  break;
                    }
                    if(inGZ)    dropBin.gz[static_cast<int>(rgnIter->second)].push_back(newParticle);
                }
            });
            

        dropBin.gzl = bgz[whichBin];    // set ghost region list for bin
        dropBin.id = whichBin;
    
        //
        //  assign random velocities within a bound
        //
        newParticle.vx = drand48()*2-1;
        newParticle.vy = drand48()*2-1;

        dropBin.content.push_front(newParticle);   
    }
    free( shuffle );
}

//
//  interact two particles
//
void apply_force( particle_t &particle, const particle_t &neighbor , double *dmin, double *davg, int *navg)
{

    double dx = neighbor.x - particle.x;
    double dy = neighbor.y - particle.y;
    double r2 = dx * dx + dy * dy;
    if( r2 > cutoff*cutoff )
        return;
    
	if (r2 != 0)
        {
	   if (r2/(cutoff*cutoff) < *dmin * (*dmin))
	      *dmin = sqrt(r2)/cutoff;
           (*davg) += sqrt(r2)/cutoff;
           (*navg) ++;
        }
		
    r2 = fmax( r2, min_r*min_r );
    double r = sqrt( r2 );
 
    
	
    //
    //  very simple short-range repulsive force
    //
    double coef = ( 1 - cutoff / r ) / r2 / mass;
    particle.ax += coef * dx;
    particle.ay += coef * dy;

    return;
    if(particle.id != neighbor.id)
    {
        cout << "Interaction\t " << particle << endl;
        cout << "\t\t " << neighbor << endl;
    }

}

//
//  integrate the ODE
//
void move( particle_t &p )
{
    //
    //  slightly simplified Velocity Verlet integration
    //  conserves energy better than explicit Euler method
    //
    p.vx += p.ax * dt;
    p.vy += p.ay * dt;
    p.x  += p.vx * dt;
    p.y  += p.vy * dt;

    //
    //  bounce from walls
    //
    while( p.x < 0 || p.x > size )
    {
        p.x  = p.x < 0 ? -p.x : 2*size-p.x;
        p.vx = -p.vx;
//        if(p.id == 3)   cout << "Bounce H:  id: " << p << endl;
    }
    while( p.y < 0 || p.y > size )
    {
        p.y  = p.y < 0 ? -p.y : 2*size-p.y;
        p.vy = -p.vy;
  //      if(p.id == 3)   cout << "Bounce V:  id: " << p << endl;
    }
}

//
//  I/O routines
//
void save( FILE *f, int n, const Mesh &world )
{
    static bool first = true;
    if( first )
    {
        fprintf( f, "%d %g\n", n, size );
        first = false;
    }
    for_each(world.begin(), world.end(),
        [f](const Bin& b)
        {
            for_each(b.content.begin(), b.content.end(), 
            [f](const particle_t &p)
            {
                fprintf( f, "%g %g\n", p.x, p.y );
            });
        });
}

//
//  command line option processing
//
int find_option( int argc, char **argv, const char *option )
{
    for( int i = 1; i < argc; i++ )
        if( strcmp( argv[i], option ) == 0 )
            return i;
    return -1;
}

int read_int( int argc, char **argv, const char *option, int default_value )
{
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return atoi( argv[iplace+1] );
    return default_value;
}

char *read_string( int argc, char **argv, const char *option, char *default_value )
{
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return argv[iplace+1];
    return default_value;
}

// old functions
void save( FILE *f, int n, particle_t *p )
{
    static bool first = true;
    if( first )
    {
        fprintf( f, "%d %g\n", n, size );
        first = false;
    }
    for( int i = 0; i < n; i++ )
        fprintf( f, "%g %g\n", p[i].x, p[i].y );
}
void init_particles( int n, particle_t *p )
{
    srand48( time( NULL ) );
        
    int sx = (int)ceil(sqrt((double)n));
    int sy = (n+sx-1)/sx;
    
    int *shuffle = (int*)malloc( n * sizeof(int) );
    for( int i = 0; i < n; i++ )
        shuffle[i] = i;
    
    for( int i = 0; i < n; i++ ) 
    {
        //
        //  make sure particles are not spatially sorted
        //
        int j = lrand48()%(n-i);
        int k = shuffle[j];
        shuffle[j] = shuffle[n-i-1];
        
        //
        //  distribute particles evenly to ensure proper spacing
        //
        p[i].x = size*(1.+(k%sx))/(1+sx);
        p[i].y = size*(1.+(k/sx))/(1+sy);

        //
        //  assign random velocities within a bound
        //
        p[i].vx = drand48()*2-1;
        p[i].vy = drand48()*2-1;
    }
    free( shuffle );
}
