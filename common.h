#ifndef __CS456_COMMON_H__
#define __CS456_COMMON_H__

#include <vector>
#include <list>
#include <map>



inline int min( int a, int b ) { return a < b ? a : b; }
inline int max( int a, int b ) { return a > b ? a : b; }

//
//  saving parameters
//
const int NSTEPS = 1000;
const int SAVEFREQ = 10;

//
// particle data structure
//
typedef struct 
{
  double x;
  double y;
  double vx;
  double vy;
  double ax;
  double ay;
} particle_t;

enum class GhostZoneRegion : char   
{
    topLeft, topRight, bottomLeft, bottomRight,
    top, left, bottom, right
};
using WallRegion = GhostZoneRegion;
using GZR = GhostZoneRegion;    // shorthand qualifier
using WR = GhostZoneRegion;    // shorthand qualifier

using NeighborRegion = std::vector<std::pair<short /* the neighbor bin */, GhostZoneRegion>>;    // visible neighbor regions for current bin
using WallRegionList = std::pair<short, std::vector<WallRegion>>;
using BinNeighbor = std::map<short, std::vector<NeighborRegion>>;    // neighbor regions by number of bins    
using BinWalls = std::map<short, std::vector<WallRegionList>>;



using Bin = std::list<particle_t>;
using Mesh = std::vector<Bin>;


//
//  timing routines
//
double read_timer( );

//
//  simulation routines
//
void set_size( int n );
void init_particles( int n, Mesh &p, short numRowBins = 1, short numColBins = 1 );
void apply_force( particle_t &particle, const particle_t &neighbor , double *dmin, double *davg, int *navg);
void move( particle_t &p );


//
//  I/O routines
//
FILE *open_save( char *filename, int n );
void save( FILE *f, int n, const Mesh& );

//
//  argument processing routines
//
int find_option( int argc, char **argv, const char *option );
int read_int( int argc, char **argv, const char *option, int default_value );
char *read_string( int argc, char **argv, const char *option, char *default_value );

#endif
