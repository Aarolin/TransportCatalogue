#include "transport_catalogue.h"

namespace transport_catalogue {

    void TransportCatalogue::AddStop(const std::string& stop, Coordinates coordinates) {

        stops_list_.push_back({ stop, std::move(coordinates) });
        auto& last_stop = stops_list_.back();
        stops_.insert({ last_stop.stop_name, &last_stop });
        stops_to_routes_.insert({ &last_stop, {} });

    }

    void TransportCatalogue::AddRoute(const std::string& route, const std::vector<std::string>& stops, RouteType route_type) {

        std::vector<Stop*> route_stops;

        for (const std::string& stop : stops) {
            route_stops.push_back(stops_.at(stop));
        }

        route_list_.push_back({ route, route_type, std::move(route_stops) });
        auto& last_route = route_list_.back();
        routes_.insert({ last_route.route_name, &last_route });

        for (auto stop : last_route.stops) {
            stops_to_routes_.at(stop).insert(last_route.route_name);
        }

    }

    const Route* TransportCatalogue::GetRoute(const std::string& route) const {

        if (routes_.count(route) == 0) {
            return nullptr;
        }

        return static_cast<const Route*>(routes_.at(route));
    }

    const Stop* TransportCatalogue::GetStop(const std::string& stop) const {

        if (stops_.count(stop) == 0) {
            return nullptr;
        }

        return static_cast<const Stop*>(stops_.at(stop));
    }

    std::optional<RouteInformation> TransportCatalogue::GetRouteInformation(const std::string& route) const {

        if (routes_.count(route) == 0) {
            return std::nullopt;
        }

        Route* finded_route = routes_.at(route);
        size_t stops_count = finded_route->stops.size();
        const std::vector<Stop*> route_stops = finded_route->stops;

        size_t route_length = 0;
        double geographical_distance = 0.0;

        for (size_t i = 0; i < stops_count - 1; ++i) {

            geographical_distance += ComputeDistance(route_stops[i]->coordinates, route_stops[i + 1]->coordinates);
            route_length += GetDistanceBetweenStops(route_stops[i], route_stops[i + 1]);

        }

        if (finded_route->route_type == RouteType::Forward) {

            for (size_t i = stops_count - 1; i > 0; --i) {
                route_length += GetDistanceBetweenStops(route_stops[i], route_stops[i - 1]);
            }

            geographical_distance *= 2;
            stops_count *= 2;
            stops_count -= 1;

        }


        std::unordered_set<Stop*> unique_stops(route_stops.begin(), route_stops.end());
        size_t unique_stops_count = unique_stops.size();
        double curvature = route_length / geographical_distance;


        return  RouteInformation{ stops_count, unique_stops_count, route_length, curvature };

    }

    std::optional<const std::set<std::string_view>> TransportCatalogue::GetStopInformation(const std::string& stop) const {

        if (stops_.count(stop) == 0) {
            return std::nullopt;
        }

        Stop* searched_stop = stops_.at(stop);
        return stops_to_routes_.at(searched_stop);

    }

    void TransportCatalogue::SetDistanceBetweenStops(const std::string& stop_from, const std::string& stop_to, size_t distance) {

        if (stops_.count(stop_from) == 0 || stops_.count(stop_to) == 0) {
            return;
        }

        stops_distances_.insert({ std::make_pair(stops_.at(stop_from), stops_.at(stop_to)), distance });

    }

    size_t TransportCatalogue::GetDistanceBetweenStops(Stop* stop_from, Stop* stop_to) const {

        auto search_key = std::make_pair(stop_from, stop_to);

        if (stops_distances_.count(search_key) == 0) {

            auto another_search_key = std::make_pair(stop_to, stop_from);

            if (stops_distances_.count(another_search_key) == 0) {
                return 0;
            }

            return stops_distances_.at(another_search_key);
        }

        return stops_distances_.at(search_key);

    }

}
