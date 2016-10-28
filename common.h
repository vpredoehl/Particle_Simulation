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
//typedef struct 
struct particle_t
{
  double x;
  double y;
  double vx;
  double vy;
  double ax;
  double ay;
  
  short id;
  particle_t()  {   id = cnt++; }
  static short cnt;
};//particle_t;

enum class GhostZoneRegion : char   
{
    top, left, bottom, right,
    num_walls,
    topLeft, topRight, bottomLeft, bottomRight,
};
using GZR = GhostZoneRegion;    // shorthand qualifier

using NeighborRegionList = vector<std::pair<short /* the neighbor bin */, GhostZoneRegion>>;    // visible neighbor regions for current bin

using BinGhostZoneList = vector<vector<GhostZoneRegion>>;
using GhostZoneListByThread = map<short /* numThreads */, BinGhostZoneList>;

struct Bin
{
    forward_list<particle_t> content;
    vector<vector<particle_t>> gz{static_cast<int>(GhostZoneRegion::num_walls)};
    vector<particle_t> crossovers;
    vector<GhostZoneRegion> gzl;    // ghost region list for this bin
    
    float leftWall, rightWall, topWall, bottomWall;
    short binToLeft, binToRight, binToTop, binToBottom, binToUpperLeft, binToUpperRight, binToLowerLeft, binToLowerRight;   // adjacent bin indexes
    
    short id;

		// methods to dynamically calculate ghost zone region contents in bin corners
	vector<particle_t> topLeftGZ() const;
	vector<particle_t> topRightGZ() const;
	vector<particle_t> bottomLeftGZ() const;
	vector<particle_t> bottomRightGZ() const;
};

using Mesh = vector<Bin>;

vector<NeighborRegionList> NeighborGZListFromBinGZList(const Mesh&);

//
//  timing routines
//
double read_timer( );

//
//  simulation routines
//
void set_size( int n );
void init_particles( int n, Mesh &p, particle_t* = 0 );
void init_particles( int n, particle_t *p );
void apply_force( particle_t &particle, const particle_t &neighbor , double *dmin, double *davg, int *navg);
void move( particle_t &p );


//
//  I/O routines
//
FILE *open_save( char *filename, int n );
void save( FILE *f, int n, particle_t *p );
void save( FILE *f, int n, const Mesh& );

//
//  argument processing routines
//
int find_option( int argc, char **argv, const char *option );
int read_int( int argc, char **argv, const char *option, int default_value );
char *read_string( int argc, char **argv, const char *option, char *default_value );


//
//  tuned constants
//
#define density 0.0005
#define mass    0.01
#define cutoff  0.01
#define min_r   (cutoff/100)
#define dt      0.0005

#endif
