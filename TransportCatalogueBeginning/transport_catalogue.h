#pragma once

#include "geo.h"

#include <algorithm>
#include <deque>
#include <functional>
#include <numeric>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace transport_catalogue {

    enum class RouteType
    {
        Circle,
        Forward
    };

    struct Stop {

        Coordinates coordinates;

        bool operator<(const Stop& rhs) const {
            return coordinates < rhs.coordinates;
        }

    };

    struct RouteInformation {

        size_t stops_count;
        size_t unique_stops_count;
        size_t route_length;
        double curvature;

    };

    struct Route {

        RouteInformation information;
        std::deque<Stop*> stops;

    };

    struct StopsHasher {

        size_t operator()(std::pair<Stop*, Stop*> stops) const {

            size_t stop_from_hash = ptr_hasher_(stops.first);
            size_t stop_to_hash = ptr_hasher_(stops.second);
            size_t result_hash = stop_from_hash + stop_to_hash * 74;
            return result_hash;

        }

    private:

        std::hash<Stop*> ptr_hasher_;
    };

    class TransportCatalogue {

    public:

        void AddStop(const std::string& stop, double latitude, double longitude);

        void AddRoute(const std::string& route, const std::vector<std::string>& stops, RouteType route_type);

        const Route* GetRoute(const std::string& route) const;

        const Stop* GetStop(const std::string& stop) const;

        const RouteInformation* GetRouteInformation(const std::string& route) const;

        std::optional<const std::set<std::string>> GetStopInformation(const std::string& stop) const;

        void AddDistanceToOtherStop(const std::pair<const std::string&, const std::string&> from_to_pair, size_t distance);

        size_t GetDistanceBetweenStops(Stop* stop_from, Stop* stop_to) const;

    private:

        std::unordered_map<std::string, Route> routes_;
        std::unordered_map<std::string, Stop> stops_;
        std::unordered_map<const Stop*, std::set<std::string>> stops_to_routes_;
        std::unordered_map<std::pair<Stop*, Stop*>, size_t, StopsHasher> stops_distances_;

    };


}
