#include "provided.h"
#include "ExpandableHashMap.h"
#include <string>
#include <vector>
#include <functional>
#include <fstream>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<std::string>()(g.latitudeText + g.longitudeText);
}

unsigned int hasher(const string& str)
{
    return std::hash<std::string>()(str);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    ExpandableHashMap <GeoCoord, vector<StreetSegment>> m_hashMap;
    vector<vector<StreetSegment>*> m_streetSeg;
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
    for (size_t k = 0; k < m_streetSeg.size(); k++)
        delete m_streetSeg[k];
}

bool StreetMapImpl::load(string mapFile)
{
    ifstream myfile(mapFile);
    if (myfile.is_open())
    {
        string line;
        string streetName;
        int numSegmentsLeft = 0;
        while(getline(myfile, line))
        {
            if (numSegmentsLeft > 0)
            {
                //process the current streetSegment
                int space_1_index = line.find(' ');
                string startLat = line.substr(0, space_1_index);
                int space_2_index = line.find(' ', space_1_index + 1);
                string startLong = line.substr(space_1_index + 1, space_2_index - space_1_index - 1);
                GeoCoord B(startLat, startLong);    //starting coordinate

                int space_3_index = line.find(' ', space_2_index + 1);
                string endLat = line.substr(space_2_index + 1, space_3_index - space_2_index - 1);
                string endLong = line.substr(space_3_index + 1, line.size() - space_3_index - 1);
                GeoCoord E(endLat, endLong);    //ending coordinate

                vector<StreetSegment>* ptrToStrSeg = m_hashMap.find(B);
                StreetSegment S(B, E, streetName);
                if (ptrToStrSeg != nullptr)
                    ptrToStrSeg->push_back(S);
                else
                {
                    vector<StreetSegment>* strSeg_B = new vector<StreetSegment>;
                    strSeg_B->push_back(S);
                    m_hashMap.associate(B, *strSeg_B);
                    m_streetSeg.push_back(strSeg_B);
                }

                //the reverse street segment
                ptrToStrSeg = m_hashMap.find(E);
                StreetSegment R(E, B, streetName);
                if (ptrToStrSeg != nullptr)
                    ptrToStrSeg->push_back(R);
                else
                {
                    vector<StreetSegment>* strSeg_E = new vector<StreetSegment>;
                    strSeg_E->push_back(R);
                    m_hashMap.associate(E, *strSeg_E);
                    m_streetSeg.push_back(strSeg_E);
                }
                numSegmentsLeft--;
            }
            else
            {
                streetName = line;
                getline(myfile, line);
                numSegmentsLeft = stoi(line);
            }
        }
        myfile.close();
        return true;
    }
    return false;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    const vector<StreetSegment>* ptrToStrSeg = m_hashMap.find(gc);
    if (ptrToStrSeg == nullptr)
        return false;
    segs = *ptrToStrSeg;
    return true;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    return m_impl->getSegmentsThatStartWith(gc, segs);
}
