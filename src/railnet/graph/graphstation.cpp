#include "graphstation.h"
#include "data/rail/railstation.h"

GraphStation::GraphStation(const RailStation &station):
    name(station.name),level(station.level),
    passenger(station.passenger),freight(station.freight),
    tracks(station.tracks)
{

}
