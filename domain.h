#pragma once

#include "geo.h"
#include <string>
#include <vector>

enum class BusType
{
    Circle,
    Forward
};

struct Stop {

    std::string stop_name;
    geo::Coordinates coordinates;

    bool operator<(const Stop& rhs) const;

};

struct BusInformation {

    size_t stops_count;
    size_t unique_stops_count;
    size_t route_length;
    double curvature;

};

struct Bus {

    std::string bus_name;
    BusType type;
    std::vector<Stop*> stops;

};

struct RouteSettings {

    int bus_wait_time = 0;
    int bus_velocity = 0;

};

struct StopsHasher {

    size_t operator()(std::pair<Stop*, Stop*> stops) const;

private:

    std::hash<Stop*> ptr_hasher_;
};

struct WayInfo {

    std::string_view bus_name;
    int stop_count;
    double weight = 0.0;

    bool operator<(const WayInfo& rhs) const;
    bool operator>(const WayInfo& rhs) const;
    WayInfo operator+(const WayInfo& rhs) const;
};

