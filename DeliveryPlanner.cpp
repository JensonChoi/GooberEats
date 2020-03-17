#include "provided.h"
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* m_streetMap;
    PointToPointRouter m_router;
    DeliveryOptimizer m_optimizer;

    DeliveryResult createCommands(const GeoCoord& start, const GeoCoord& dest,
        vector<DeliveryCommand>& commands, double& totalDist, string item) const;

    string getDirection(const StreetSegment& street) const;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
    :m_streetMap(sm), m_router(sm), m_optimizer(sm)
{
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    //if there is no deliveries
    if (deliveries.size() == 0)
        return DELIVERY_SUCCESS;

    //optimize order
    double distOld, distNew;
    vector<DeliveryRequest> copyOfDeliveries;
    for (size_t k = 0; k < deliveries.size(); k++)
    {
        copyOfDeliveries.push_back(deliveries[k]);
    }
    m_optimizer.optimizeDeliveryOrder(depot, copyOfDeliveries, distOld, distNew);

    //find route
    double distSegment;
    totalDistanceTravelled = 0;
    list<StreetSegment> route;
    vector<DeliveryCommand> output;
    DeliveryResult res = createCommands(depot, copyOfDeliveries[0].location, output, 
        distSegment, copyOfDeliveries[0].item);
    if (res != DELIVERY_SUCCESS)
        return res;
    totalDistanceTravelled += distSegment;

    for (size_t k = 1; k < copyOfDeliveries.size(); k++)
    {
        res = createCommands(copyOfDeliveries[k - 1].location, copyOfDeliveries[k].location, output,
            distSegment, copyOfDeliveries[k].item);
        if (res != DELIVERY_SUCCESS)
            return res;
        totalDistanceTravelled += distSegment;
    }

    res = createCommands(copyOfDeliveries[copyOfDeliveries.size() - 1].location, depot, output, 
        distSegment, "");
    if (res != DELIVERY_SUCCESS)
        return res;
    totalDistanceTravelled += distSegment;
    swap(commands, output);
    return DELIVERY_SUCCESS;
}

DeliveryResult DeliveryPlannerImpl::createCommands(const GeoCoord& start, const GeoCoord& dest,
    vector<DeliveryCommand>& commands, double& totalDist, string item) const
{
    //assume it is valid to just append onto commands, no need to reset
    list<StreetSegment> route;
    DeliveryResult output = m_router.generatePointToPointRoute(start, dest, route, totalDist);

    //analyze the route
    StreetSegment currStr = *(route.begin());   //initialize as the first street segment
    bool shouldGenerateNewProceed = true;
    for (list<StreetSegment>::iterator it = route.begin(); it != route.end(); it++)
    {
        DeliveryCommand myCommand;
        if (it->name != currStr.name)
        {
            //going onto a new street
            double angle = angleBetween2Lines(currStr, *it);
            if (angle >= 1 && angle < 180)
            {
                myCommand.initAsTurnCommand("left", it->name);
            }
            else if (angle >= 180 && angle <= 359)
            {
                myCommand.initAsTurnCommand("right", it->name);
            }
            commands.push_back(myCommand);
            currStr = *it;
            shouldGenerateNewProceed = true;
        }
        if (shouldGenerateNewProceed)
        {
            //generate a proceed command
            string direction = getDirection(*it);
            myCommand.initAsProceedCommand(direction, it->name, distanceEarthMiles(it->start, it->end));
            commands.push_back(myCommand);
            shouldGenerateNewProceed = false;
        }
        else
        {
            //increase the distance of the previous proceed command
            commands[commands.size() - 1].increaseDistance(distanceEarthMiles(it->start, it->end));
        }
    }

    //delivering the item
    if (item.length() != 0)
    {
        DeliveryCommand delivery;
        delivery.initAsDeliverCommand(item);
        commands.push_back(delivery);
    }
    return output;
}

string DeliveryPlannerImpl::getDirection(const StreetSegment& street) const
{
    double angle = angleOfLine(street);
    string output;
    if (angle >= 0 && angle < 22.5)
        output = "east";
    else if (angle >= 22.5 && angle < 67.5)
        output = "northeast";
    else if (angle >= 67.5 && angle < 112.5)
        output = "north";
    else if (angle >= 112.5 && angle < 157.5)
        output = "northwest";
    else if (angle >= 157.5 && angle < 202.5)
        output = "west";
    else if (angle >= 202.5 && angle < 247.5)
        output = "southwest";
    else if (angle >= 247.5 && angle < 292.5)
        output = "south";
    else if (angle >= 292.5 && angle < 337.5)
        output = "southeast";
    else if (angle >= 337.5)
        output = "east";
    return output;
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
