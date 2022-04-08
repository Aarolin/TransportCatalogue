#pragma once

#include "domain.h"

#include <algorithm>
#include <deque>
#include <functional>
#include <map>
#include <numeric>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>


namespace transport_catalogue {


    class TransportCatalogue {

    public:

        void AddStop(const std::string& stop, geo::Coordinates coordinates);
        void AddRoute(const std::string& route, const std::vector<std::string>& stops, RouteType route_type);

        const Route* GetRoute(const std::string& route) const;
        const Stop* GetStop(const std::string& stop) const;

        std::optional<RouteInformation> GetRouteInformation(const std::string& route) const;
        std::optional<const std::set<std::string_view>> GetStopInformation(const std::string& stop) const;

        void SetDistanceBetweenStops(const std::string& stop_from, const std::string& stop_to, size_t distance);
        size_t GetDistanceBetweenStops(Stop* stop_from, Stop* stop_to) const;

        const std::map<std::string_view, Route*>& GetAllRoutes() const;

    private:

        std::map<std::string_view, Route*> routes_;
        std::unordered_map<std::string_view, Stop*> stops_;
        std::unordered_map<Stop*, std::set<std::string_view>> stops_to_routes_;
        std::unordered_map<std::pair<Stop*, Stop*>, size_t, StopsHasher> stops_distances_;
        std::deque<Stop> stops_list_;
        std::deque<Route> route_list_;

    };


}
