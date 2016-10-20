#include "logging.h"

using namespace std;

ostream& operator<<(ostream& o, particle_t t)   {   o << "id: " << t.id << " a: ( " << t.ax << ", " << t.ay << " )" << " v: ( " << t.vx << ", " << t.vy << " )" << " ( " << t.x << ", " << t.y << " )"; return o;  }
bool operator==(particle_t p1, particle_t p2)   {   return p1.id == p2.id && p1.ax == p2.ax && p1.ay == p2.ay && p1.vx == p2.vx && p1.vy == p2.vy;  }

ostream& operator<<(ostream& o, const Bin& b)
{
    cout << "Content: " << endl << b.content << endl;
    if(b.crossovers.size()) cout << "Crossovers: " << endl << b.crossovers << endl;
    if(b.gz[static_cast<int>(GZR::left)].size())    cout << "left gz: " << endl << b.gz[static_cast<int>(GZR::left)] << endl;
    if(b.gz[static_cast<int>(GZR::right)].size())   cout << "right gz: " << endl << b.gz[static_cast<int>(GZR::right)] << endl;
    if(b.gz[static_cast<int>(GZR::top)].size())   cout << "top gz: " << endl << b.gz[static_cast<int>(GZR::top)] << endl;
    if(b.gz[static_cast<int>(GZR::bottom)].size())   cout << "bottom gz: " << endl << b.gz[static_cast<int>(GZR::bottom)] << endl;
    return o;
}


// SerialRunTest

SerialRunTest::SerialRunTest(const Mesh &m)
    : srtWorld { m.cbegin()->content.cbegin(), m.cbegin()->content.cend()}
{
        // combine contents of each bin into SerialRunTest::world
    for_each(next(m.cbegin()), m.cend(), [this](const Bin &b)
        {
            srtWorld.insert_after(srtWorld.begin(), b.content.cbegin(), b.content.cend());
        });
}

void SerialRunTest::interact(ApplyForceFunct interact)
{
    auto particleIter = srtWorld.begin();
    while(particleIter != srtWorld.end())
    {
        auto& p1 = *particleIter;

        p1.ax = p1.ay = 0;
        for_each(++particleIter, srtWorld.end(), [this, &p1, interact](const particle_t &neighbor)
            {
                interact(p1, neighbor, &dmin,&davg,&navg);
            });
    }
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
