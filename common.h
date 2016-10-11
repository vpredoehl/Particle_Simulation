#ifndef __CS456_COMMON_H__
#define __CS456_COMMON_H__

#include <vector>
#include <forward_list>
#include <map>

using std::map;
using std::vector;
using std::forward_list;

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

using NeighborRegion = vector<std::pair<short /* the neighbor bin */, GhostZoneRegion>>;    // visible neighbor regions for current bin
using WallRegionList = std::pair<short, vector<WallRegion>>;
using BinNeighbor = map<short, vector<NeighborRegion>>;    // neighbor regions by number of bins    
using BinWalls = map<short, vector<WallRegionList>>;


using Bin = struct
{
    forward_list<particle_t> content;
};

using Mesh = vector<Bin>;


//
//  timing routines
//
double read_timer( );

//
//  simulation routines
//
void set_size( int n );
void init_particles( int n, Mesh &p );
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
