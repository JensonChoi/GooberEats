#include "provided.h"
#include <vector>
using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private:
    const StreetMap* m_streetMap;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
    :m_streetMap(sm)
{
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    oldCrowDistance = 0;
    newCrowDistance = 0;
    //check edge case
    if (deliveries.size() == 0)
        return;

    //compute oldCrowDistance
    oldCrowDistance = distanceEarthMiles(depot, deliveries[0].location);
    for (size_t k = 1; k < deliveries.size(); k++)
    {
        oldCrowDistance += distanceEarthMiles(deliveries[k - 1].location, deliveries[k].location);
    }
    oldCrowDistance += distanceEarthMiles(deliveries[deliveries.size() - 1].location, depot);
    
    //do some reordering
    int minI = 0;
    double minDistFromDepot = distanceEarthMiles(depot, deliveries[0].location);
    for (size_t k = 1; k < deliveries.size(); k++)
    {
        double currDist = distanceEarthMiles(depot, deliveries[k].location);
        if (currDist < minDistFromDepot)
        {
            minDistFromDepot = currDist;
            minI = k;
        }
    }
    //swap the closest request from the depot to the front of the delivery requests
    swap(deliveries[0], deliveries[minI]);

    for (size_t i = 0; i < deliveries.size() - 1; i++)
    {
        int minIndex = i + 1;
        double minDist = distanceEarthMiles(deliveries[i].location, deliveries[minIndex].location);
        for (size_t j = i + 2; j < deliveries.size(); j++)
        {
            double currDist = distanceEarthMiles(deliveries[i].location, deliveries[j].location);
            if (currDist < minDist)
            {
                minDist = currDist;
                minIndex = j;
            }
        }
        //swap in a way to minimize the distance between deliveries[i] & deliveries[i + 1]
        swap(deliveries[i + 1], deliveries[minIndex]);
    }
    
    //compute newCrowDistance
    newCrowDistance = distanceEarthMiles(depot, deliveries[0].location);
    for (size_t k = 1; k < deliveries.size(); k++)
    {
        newCrowDistance += distanceEarthMiles(deliveries[k - 1].location, deliveries[k].location);
    }
    newCrowDistance += distanceEarthMiles(deliveries[deliveries.size() - 1].location, depot);
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
