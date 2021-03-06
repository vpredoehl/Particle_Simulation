#include "logging.h"

using namespace std;

ostream& operator<<(ostream& o, particle_t t)   {   o << "id: " << t.id << " a: ( " << t.ax << ", " << t.ay << " )" << " v: ( " << t.vx << ", " << t.vy << " )" << " ( " << t.x << ", " << t.y << " )"; return o;  }
bool operator==(particle_t p1, particle_t p2)   {   return p1.id == p2.id && p1.ax == p2.ax && p1.ay == p2.ay && p1.vx == p2.vx && p1.vy == p2.vy && p1.x == p2.x && p1.y == p2.y;  }

ostream& operator<<(ostream& o, const Bin& b)
{
    if(LogLevel(LL::content))   cout << "Content: " << endl << b.content << endl;
    if(LogLevel(LL::crossover) && b.crossovers.size()) cout << "Crossovers: " << endl << b.crossovers << endl;
    if(LogLevel(LL::gz) && b.gz[static_cast<int>(GZR::left)].size())    cout << "left gz: " << endl << b.gz[static_cast<int>(GZR::left)] << endl;
    if(LogLevel(LL::gz) && b.gz[static_cast<int>(GZR::right)].size())   cout << "right gz: " << endl << b.gz[static_cast<int>(GZR::right)] << endl;
    if(LogLevel(LL::gz) && b.gz[static_cast<int>(GZR::top)].size())   cout << "top gz: " << endl << b.gz[static_cast<int>(GZR::top)] << endl;
    if(LogLevel(LL::gz) && b.gz[static_cast<int>(GZR::bottom)].size())   cout << "bottom gz: " << endl << b.gz[static_cast<int>(GZR::bottom)] << endl;
    return o;
}

ostream& operator<<(ostream& o, const vector<NeighborRegionList>& nrl)
{
    short id = 0;   // bin id is the position in the container
    
    for_each(nrl.cbegin(), nrl.cend(), [&o,&id](NeighborRegionList l)
             {
                 o << "bin id: " << id++ << endl << l << endl;
             });
    return o;
}


// SerialRunTest

SerialRunTest::SerialRunTest(const Mesh &m)
    : srtWorld { m.cbegin()->content.cbegin(), m.cbegin()->content.cend()}
{
        // combine contents of each bin into SerialRunTest::world
    for_each(next(m.cbegin()), m.cend(), [this](const Bin &b)
        {
            copy(b.content.cbegin(), b.content.cend(), front_inserter(srtWorld));
        });
    srtWorld.sort();
}

void SerialRunTest::interact(ApplyForceFunct interact)
{
    for(auto particleIter = srtWorld.begin(); particleIter != srtWorld.end(); particleIter++)
    {
        auto& p1 = *particleIter;

        p1.ax = p1.ay = 0;
        for_each(srtWorld.cbegin(), srtWorld.cend(), [this, &p1, interact](const particle_t &neighbor)
            {
                interact(p1, neighbor, &dmin,&davg,&navg);
            });
    }
}

bool SerialRunTest::operator!=(const particle_t *particles) const
{
    short n = list_size(srtWorld);
    bool fail = false;
    
    for(int i = 0; i < n; i++)
    {
        auto findIter = find_if(srtWorld.cbegin(), srtWorld.cend(),
            [=](particle_t p)    {   return p.id == particles[i].id; });
       
        if(findIter != srtWorld.cend() && *findIter != particles[i])
        {
            cout << "Stock code diverged " << particles[i] << endl << "\t\tsrt " << *findIter << endl;
            fail = true;
        }
    }
    return fail;
}

bool SerialRunTest::operator!=(const Mesh &w) const
{
    map<short, particle_t> w2;
    particle_t p2;
    
    for_each(w.cbegin(), w.cend(), [&w2](const Bin &b)
        {
            for_each(b.content.cbegin(), b.content.cend(), [&w2](particle_t p)
            {
                w2[p.id] = p;
            });
        });

    bool fail = false;
    for(auto diffIter = srtWorld.cbegin(); diffIter != srtWorld.cend(); diffIter++)    
        if(auto findIter = w2.find(diffIter->id) != w2.cend())
        {
            auto p1 = *diffIter;
            p2 = w2[diffIter->id];
            if(*diffIter != p2)
            {
                cout << "failed: srt " << *diffIter << endl << "\t and " << p2 << endl;
                fail = true;
            }
        }
    
    return fail;
}

bool operator!=(const Mesh &world, const particle_t *particles)
{
    vector<particle_t> w;
    
        // copy contents of Mesh world to w
    for_each(world.cbegin(), world.cend(), [&w](const Bin &b)   {   copy(b.content.cbegin(), b.content.cend(), back_inserter(w));   });
    sort(w.begin(), w.end());
    
    bool eq = true;
    vector<particle_t>::const_iterator worldIter;
    for(worldIter = w.cbegin(); worldIter != w.cend(); worldIter++)
    {
        auto p1 = *worldIter;
        auto p2 = particles[p1.id];
        
        if(p1.id != p2.id)
        {
            cout << "world: " << p1 << endl << " != stock: " << p2 << endl;
            eq = false;
        }
    }
    return !eq;
}
