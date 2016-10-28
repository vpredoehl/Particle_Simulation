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

unsigned long numThreads;
double size;
short binsPerRow, binsPerCol;
short particle_t::cnt = 0;


GhostZoneListByThread binGZList   // enumerates each bin's ghost zone regions
    {
            //      layout for two bins
            //  _________________________
            //  |           |           |
            //  |           |           |
            //  |     0     |     1     |
            //  |           |           |
            //  |___________|___________|
        
        { 2,    // two threads
            {
                {GZR::right}, // bin 0
                {GZR::left} // bin 1
            }
        },
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

        { 4,    // four threads
            {
                    // ghost zone regions by bin
                {GZR::right, GZR::bottom, GZR::bottomRight},  // bin 0
                {GZR::left, GZR::bottom, GZR::bottomLeft},   // bin 1
                {GZR::right, GZR::top, GZR::topRight},     // bin 2
                {GZR::left, GZR::top, GZR::topLeft}       // bin 3
            }
        },
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
        
        { 8,    // eight threads
            {
                    // ghost zone regions by bin
                {GZR::right, GZR::bottom, GZR::bottomRight},                                // bin 0
                {GZR::left, GZR::bottomLeft, GZR::bottom, GZR::bottomRight, GZR::right},    // bin 1
                {GZR::left, GZR::bottomLeft, GZR::bottom, GZR::bottomRight, GZR::right},    // bin 2
                {GZR::left, GZR::bottom, GZR::bottomLeft},                                  // bin 3
                {GZR::right, GZR::top, GZR::topRight},                                      // bin 4
                {GZR::left, GZR::topLeft, GZR::top, GZR::topRight, GZR::right},             // bin 5
                {GZR::left, GZR::topLeft, GZR::top, GZR::topRight, GZR::right},             // bin 6
                {GZR::left, GZR::top, GZR::topLeft}                                         // bin 7
            }
        },
        
            //      layout for sixteen bins
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
            //  |           |           |           |           |
            //  |           |           |           |           |
            //  |     8     |     9     |     10    |     11    |
            //  |           |           |           |           |
            //  |___________|___________|___________|___________|
            //  |           |           |           |           |
            //  |           |           |           |           |
            //  |     12    |     13    |     14    |     15    |
            //  |           |           |           |           |
            //  |___________|___________|___________|___________|

        { 16,   // sixteen threads
            {
                {GZR::right, GZR::bottom, GZR::bottomRight},                                // bin 0
                {GZR::left, GZR::bottomLeft, GZR::bottom, GZR::bottomRight, GZR::right},    // bin 1
                {GZR::left, GZR::bottomLeft, GZR::bottom, GZR::bottomRight, GZR::right},    // bin 2
                {GZR::left, GZR::bottom, GZR::bottomLeft},                                  // bin 3
                {GZR::top, GZR::topRight, GZR::right, GZR::bottomRight, GZR::bottom},       // bin 4
                {GZR::topLeft, GZR::top, GZR::topRight, GZR::right, GZR::bottomRight, GZR::bottom, GZR::bottomLeft, GZR::left},     // bin 5
                {GZR::topLeft, GZR::top, GZR::topRight, GZR::right, GZR::bottomRight, GZR::bottom, GZR::bottomLeft, GZR::left},     // bin 6
                {GZR::bottom, GZR::bottomLeft, GZR::left, GZR::topLeft, GZR::top},          // bin 7
                {GZR::top, GZR::topRight, GZR::right, GZR::bottomRight, GZR::bottom},       // bin 8
                {GZR::topLeft, GZR::top, GZR::topRight, GZR::right, GZR::bottomRight, GZR::bottom, GZR::bottomLeft, GZR::left},     // bin 9
                {GZR::topLeft, GZR::top, GZR::topRight, GZR::right, GZR::bottomRight, GZR::bottom, GZR::bottomLeft, GZR::left},     // bin 10
                {GZR::bottom, GZR::bottomLeft, GZR::left, GZR::topLeft, GZR::top},          // bin 11
                {GZR::right, GZR::top, GZR::topRight},                                      // bin 12
                {GZR::left, GZR::topLeft, GZR::top, GZR::topRight, GZR::right},             // bin 13
                {GZR::left, GZR::topLeft, GZR::top, GZR::topRight, GZR::right},             // bin 14
                {GZR::left, GZR::top, GZR::topLeft}                                         // bin 15
            }
        }
    };
    

// specific enumeration for current number of threads
vector<NeighborRegionList> nrl;
BinGhostZoneList bgz; 


vector<NeighborRegionList> NeighborGZListFromBinGZList(const Mesh &world)
{
    vector<NeighborRegionList> nrl { numThreads };

    for_each(world.cbegin(), world.cend(), [&nrl](const Bin &b)
        {
            if(b.binToLeft != -1)
            {
                vector<GhostZoneRegion> l = bgz[b.binToLeft];
                
                    // check if bin to left has right gz
                if(l.cend() != find(l.cbegin(), l.cend(), GZR::right))
                        // and add it to this bin's neighbor gz list if it does
                   nrl[b.id].push_back({b.binToLeft, GZR::right});
            }
            if(b.binToUpperLeft != -1)
            {
                vector<GhostZoneRegion> ul = bgz[b.binToUpperLeft];
                if(ul.cend() != find(ul.cbegin(), ul.cend(), GZR::bottomRight))     nrl[b.id].push_back({b.binToUpperLeft, GZR::bottomRight});
            }
            if(b.binToTop != -1)
            {
                vector<GhostZoneRegion> t = bgz[b.binToTop];
                if(t.cend() != find(t.cbegin(), t.cend(), GZR::bottom))        nrl[b.id].push_back({b.binToTop, GZR::bottom});
            }
            if(b.binToUpperRight != -1)
            {
                vector<GhostZoneRegion> ur = bgz[b.binToUpperRight];
                if(ur.cend() != find(ur.cbegin(), ur.cend(), GZR::bottomLeft))      nrl[b.id].push_back({b.binToUpperRight, GZR::bottomLeft});
            }
            if(b.binToRight != -1)
            {
                vector<GhostZoneRegion> r = bgz[b.binToRight];
                if(r.cend() != find(r.cbegin(), r.cend(), GZR::left))   nrl[b.id].push_back({b.binToRight, GZR::left});
            }
            if(b.binToLowerRight != -1)
            {
                vector<GhostZoneRegion> lr = bgz[b.binToLowerRight];
                if(lr.cend() != find(lr.cbegin(), lr.cend(), GZR::topLeft))    nrl[b.id].push_back({b.binToLowerRight, GZR::topLeft});
            }
            if(b.binToBottom != -1)
            {
                vector<GhostZoneRegion> bot = bgz[b.binToBottom];
                if(bot.cend() != find(bot.cbegin(), bot.cend(), GZR::top))    nrl[b.id].push_back({b.binToBottom, GZR::top});
            }
            if(b.binToLowerLeft != -1)
            {
                vector<GhostZoneRegion> ll = bgz[b.binToLowerLeft];
                if(ll.cend() != find(ll.cbegin(), ll.cend(), GZR::topRight))    nrl[b.id].push_back({b.binToLowerLeft, GZR::topRight});
            }
        });
    
        // sort neighbor region list if we are logging it
    if(LogLevel(LL::neighborgzlist))    for_each(nrl.begin(), nrl.end(), [](NeighborRegionList &l)  {   sort(l.begin(), l.end());   });
    return nrl;
}

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
void init_particles( int n, Mesh &p, particle_t *particles)
{
    auto tt = time ( NULL );
    srand48( tt );
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
        for_each(nrl.begin(), nrl.end(), [=, &dropBin](const NeighborRegionList &neighbors)
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
    
        //
        //  assign random velocities within a bound
        //
        newParticle.vx = drand48()*2-1;
        newParticle.vy = drand48()*2-1;
        
        newParticle.ax = newParticle.ay = 0;

        dropBin.content.push_front(newParticle);   
        if(particles)   particles[i] = newParticle; // initialize stock code-style particle
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

    if(LogLevel(LL::interaction) && particle.id != neighbor.id)
    {
        cout << "Interaction\t " << particle << endl;
        cout << "\t\t\t" << neighbor << endl;
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
        if(LogLevel(LL::bounce))     cout << "Bounce H:  id: " << p << endl;
    }
    while( p.y < 0 || p.y > size )
    {
        p.y  = p.y < 0 ? -p.y : 2*size-p.y;
        p.vy = -p.vy;
        if(LogLevel(LL::bounce))    cout << "Bounce V:  id: " << p << endl;
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


vector<particle_t> Bin::topLeftGZ() const
{
	vector<particle_t> s;
	const vector<particle_t> &topGZ = gz[static_cast<int>(GZR::top)], &leftGZ = gz[static_cast<int>(GZR::left)];

	for_each(topGZ.cbegin(), topGZ.cend(), [&s, leftGZ](particle_t p1)	
		{
			auto findIter = find_if(leftGZ.cbegin(), leftGZ.cend(), [p1](particle_t p2)	{	return p1.id == p2.id;	});

			if(findIter != leftGZ.cend()) s.push_back(p1);	// found particle in top left gz
		});
	return s;	// compiler move semantics by default
}

vector<particle_t> Bin::topRightGZ() const
{
	vector<particle_t> s;
	const vector<particle_t> &topGZ = gz[static_cast<int>(GZR::top)], &rightGZ = gz[static_cast<int>(GZR::right)];

	for_each(topGZ.cbegin(), topGZ.cend(), [&s, rightGZ](particle_t p1)	
		{
			auto findIter = find_if(rightGZ.cbegin(), rightGZ.cend(), [p1](particle_t p2)	{	return p1.id == p2.id;	});

			if(findIter != rightGZ.cend()) s.push_back(p1);	// found particle in top left gz
		});
	return s;	// compiler move semantics by default
}

vector<particle_t> Bin::bottomLeftGZ() const
{
	vector<particle_t> s;
	const vector<particle_t> &bottomGZ = gz[static_cast<int>(GZR::bottom)], &leftGZ = gz[static_cast<int>(GZR::left)];

	for_each(bottomGZ.cbegin(), bottomGZ.cend(), [&s, leftGZ](particle_t p1)	
		{
			auto findIter = find_if(leftGZ.cbegin(), leftGZ.cend(), [p1](particle_t p2)	{	return p1.id == p2.id;	});

			if(findIter != leftGZ.cend()) s.push_back(p1);	// found particle in top left gz
		});
	return s;	// compiler move semantics by default
}

vector<particle_t> Bin::bottomRightGZ() const
{
	vector<particle_t> s;
	const vector<particle_t> &bottomGZ = gz[static_cast<int>(GZR::bottom)], &rightGZ = gz[static_cast<int>(GZR::right)];

	for_each(bottomGZ.cbegin(), bottomGZ.cend(), [&s, rightGZ](particle_t p1)	
		{
			auto findIter = find_if(rightGZ.cbegin(), rightGZ.cend(), [p1](particle_t p2)	{	return p1.id == p2.id;	});

			if(findIter != rightGZ.cend()) s.push_back(p1);	// found particle in top left gz
		});
	return s;	// compiler move semantics by default
}

