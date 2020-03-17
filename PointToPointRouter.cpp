#include "provided.h"
#include "ExpandableHashMap.h"
#include <list>
#include <queue>
#include <utility>
using namespace std;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_streetMap;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
    :m_streetMap(sm)
{
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
    const GeoCoord& start,
    const GeoCoord& end,
    list<StreetSegment>& route,
    double& totalDistanceTravelled) const
{
    vector<StreetSegment> v;

    //check if start and end are valid
    if (!m_streetMap->getSegmentsThatStartWith(start, v) || !m_streetMap->getSegmentsThatStartWith(end, v))
        return BAD_COORD;

    //Using the A* searching algorithm
    //the heuristic function is the Euclidean distance between that GeoCoord and the end
    priority_queue<pair<double, GeoCoord>, vector<pair<double, GeoCoord>>, 
        greater<pair<double, GeoCoord>>> coordToExamine;

    //push the start into the priority queue
    coordToExamine.push(make_pair(distanceEarthMiles(start, end), start));

    //the value represents the cost of getting to that GeoCoord
    ExpandableHashMap<GeoCoord, double> coordVisited;   
    coordVisited.associate(start, 0);   //dropping breadcrumbs
    ExpandableHashMap<GeoCoord, GeoCoord> locationsOfPreviousWayPoint;  //for backtracking purposes

    while (!coordToExamine.empty())
    {
        GeoCoord curr = coordToExamine.top().second;
        coordToExamine.pop();
   
        //check if we are at our destination
        if (curr == end)
        {
            list<StreetSegment> output;
            totalDistanceTravelled = 0;
            vector<StreetSegment> v1, v2;
            for(;;)
            {
                GeoCoord prev = *locationsOfPreviousWayPoint.find(curr);
                totalDistanceTravelled += distanceEarthMiles(curr, prev);

                //find the street segment that contains both curr and prev
                m_streetMap->getSegmentsThatStartWith(curr, v1);
                m_streetMap->getSegmentsThatStartWith(prev, v2);

                bool strSegFound = false;
                for (size_t i = 0; i < v1.size(); i++)
                {
                    for (size_t j = 0; j < v2.size(); j++)
                    {
                        if (v1[i].start == v2[j].end && v1[i].end == v2[j].start) //street segment found
                        {
                            output.push_front(v1[i]);
                            strSegFound = true;
                            break;
                        }
                    }
                    if (strSegFound)
                        break;
                }

                curr = prev;
                if (curr == start)  //we're done
                    break;
            }
            swap(route, output);
            return DELIVERY_SUCCESS;
        }
        m_streetMap->getSegmentsThatStartWith(curr, v);

        for (int k = 0; k < v.size(); k++)
        {
            GeoCoord next = v[k].end;
            //if we encounter a GeoCoord that we've visited, we'll just ignore it for simplicity
            if (coordVisited.find(next) != nullptr) 
                continue;

            double* ptrToCurrCost = coordVisited.find(curr);
            if (ptrToCurrCost != nullptr)
            {
                //compute the cost to next
                double cost = *ptrToCurrCost + distanceEarthMiles(curr, next);
                coordVisited.associate(next, cost);  //dropping breadcrumbs
                locationsOfPreviousWayPoint.associate(next, curr);
                //push next into the priority queue
                coordToExamine.push(make_pair(cost + distanceEarthMiles(next, end), next));
            }
        }
    }
    return NO_ROUTE;
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
    const GeoCoord& start,
    const GeoCoord& end,
    list<StreetSegment>& route,
    double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
