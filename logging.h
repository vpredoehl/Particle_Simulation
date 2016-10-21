#ifndef LOGGING_H
#define LOGGING_H

#include "common.h"
#include <iostream>
#include <algorithm>
#include <forward_list>
#include <map>
#include <functional>

using std::forward_list;
using std::ostream;
using std::vector;
using std::cout;
using std::map;
using std::function;

    // stream
ostream& operator<<(ostream&, particle_t);
ostream& operator<<(ostream&, const Bin&);

    // comparators
bool operator==(particle_t, particle_t);
inline bool operator!=(particle_t a, particle_t b)  {   return !(a == b);   }
inline bool operator<(particle_t a, particle_t b)  {   return a.id < b.id; }

template<class T>   short list_size(const forward_list<T> &l)   {   short cnt = 0;  auto b = l.begin(); while(b != l.end())  {   b++;    cnt++;  }  return cnt; }
template<class T>   ostream& operator<<(ostream& o, const vector<T> &v)    {   for_each(v.cbegin(), v.cend(), [&o](T p) {   o << p << endl; }); return o;   }
template<class T>   ostream& operator<<(ostream& o, const forward_list<T> &v)   {   for_each(v.cbegin(), v.cend(), [&o](T p)    {   o << p << endl; }); return o;   }


enum class LogFlags : short
{
    none = 0,
    content = 1,
    gz = 2,
    crossover = 4,
    interaction = 8,
    applyforce = 16,
    bounce = 32,
    serialruntest = 64,
    move = 128,
    step = 256
};
using LL = LogFlags;    // shorthand alias 

constexpr LogFlags operator|(LogFlags l1,  LogFlags l2)  {   return static_cast<LogFlags>(static_cast<int>(l1) | static_cast<int>(l2)); }
constexpr LogFlags operator&(LogFlags l1,  LogFlags l2)  {   return static_cast<LogFlags>(static_cast<int>(l1) & static_cast<int>(l2)); }
constexpr bool operator==(LogFlags l1, LogFlags l2) {   return true;    }

constexpr LogFlags ll = LL::step;//LL::content | LL::crossover | LL::gz | LL::interaction | LL::serialruntest;
constexpr bool LogLevel(LogFlags f)  {   return static_cast<bool>(ll & f);    }


using ApplyForceFunct = function<void( particle_t&, const particle_t& , double*, double*, int*)>;
class SerialRunTest // container for serial run to compare against multi-bin run
{
    int navg,nabsavg=0;
    double davg,dmin, absmin=1.0, absavg=0.0;

    public:
        forward_list<particle_t> srtWorld;

        SerialRunTest(const Mesh&);
        
        void move(function<void(particle_t&)> move) {   for_each(srtWorld.begin(), srtWorld.end(), move); }
        void interact(ApplyForceFunct);
        bool operator!=(const Mesh&)  const;
        bool operator!=(const particle_t*) const;
};


#endif