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

using namespace std;

double size;
short numThreads = 2;
short binsPerRow, binsPerCol;

BinWalls walls
        {
            { 2, {{0, {WR::top, WR::left, WR::bottom}}, {1, {WR::top, WR::right, WR::bottom}}}},
            { 4, {
                    {0, {WR::top, WR::left}},
                    {1, {WR::top, WR::right}}, 
                    {2, {WR::left, WR::bottom}}, 
                    {3, {WR::bottom, WR::right}}
                 }
            }
        };
        

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
  
vector<NeighborRegion> nr;  // for current number of bins


//
//  tuned constants
//
#define density 0.0005
#define mass    0.01
#define cutoff  0.01
#define min_r   (cutoff/100)
#define dt      0.0005

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
    srand48( time( NULL ) );
        
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
        float leftWall = x * size / binsPerRow;
        float rightWall = (x+1) * size / binsPerRow;
        float topWall = y * size / binsPerCol;
        float bottomWall = (y+1) * size/binsPerCol;
        Bin &dropBin = p[whichBin];
        
            // ghost zone placement
        auto z = nr[whichBin];  // zone list
        for_each(z.begin(), z.end(),
        [](const pair<short /* the neighbor bin */, GhostZoneRegion> &r)   
            {
                switch(r.first)
                {
                    
                }
            });
    
        //
        //  assign random velocities within a bound
        //
        newParticle.vx = drand48()*2-1;
        newParticle.vy = drand48()*2-1;

        dropBin.push_front(newParticle);   
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
    }
    while( p.y < 0 || p.y > size )
    {
        p.y  = p.y < 0 ? -p.y : 2*size-p.y;
        p.vy = -p.vy;
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
            for_each(b.begin(), b.end(), 
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
