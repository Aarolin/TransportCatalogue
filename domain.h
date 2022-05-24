#pragma once

#include "geo.h"
#include <string>
#include <vector>

enum class RouteType
{
    Circle,
    Forward
};

struct Stop {

    std::string stop_name;
    geo::Coordinates coordinates;

    bool operator<(const Stop& rhs) const;

};

struct RouteInformation {

    size_t stops_count;
    size_t unique_stops_count;
    size_t route_length;
    double curvature;

};

struct Route {

    std::string route_name;
    RouteType route_type;
    std::vector<Stop*> stops;

};

struct StopsHasher {

    size_t operator()(std::pair<Stop*, Stop*> stops) const;

private:

    std::hash<Stop*> ptr_hasher_;
};