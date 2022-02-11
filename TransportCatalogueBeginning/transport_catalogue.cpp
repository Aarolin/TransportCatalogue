#include "transport_catalogue.h"

namespace transport_catalogue {

    void TransportCatalogue::AddStop(const std::string& stop, double latitude, double longitude) {

        stops_.insert({ stop, { {latitude, longitude} } });
        const Stop& inserted_stop = stops_.at(stop);
        stops_to_routes_.insert({ &inserted_stop, {} });

    }

    void TransportCatalogue::AddRoute(const std::string& route, const std::vector<std::string>& stops, RouteType route_type) {

        std::deque<Stop*> route_stops;
        size_t stops_count = stops.size();

        for (const std::string& stop : stops) {
            route_stops.push_back(&stops_.at(stop));
        }

        size_t route_length = 0;
        double geographical_distance = 0.0;

        for (size_t i = 0; i < stops_count - 1; ++i) {

            geographical_distance += ComputeDistance(route_stops[i]->coordinates, route_stops[i + 1]->coordinates);
            route_length += GetDistanceBetweenStops(route_stops[i], route_stops[i + 1]);

        }

        if (route_type == RouteType::Forward) {

            for (size_t i = stops_count - 1; i > 0; --i) {
                route_length += GetDistanceBetweenStops(route_stops[i], route_stops[i - 1]);
            }

            geographical_distance *= 2;
            stops_count *= 2;
            stops_count -= 1;

        }


        std::set<std::string_view> unique_stops(stops.begin(), stops.end());
        size_t unique_stops_count = unique_stops.size();
        double curvature = route_length / geographical_distance;
        routes_.insert({ route, { { stops_count, unique_stops_count, route_length, curvature },  std::move(route_stops) } });

        const Route& added_route = routes_.at(route);

        for (const Stop* stop : added_route.stops) {
            stops_to_routes_.at(stop).insert(route);
        }

    }

    const Route* TransportCatalogue::GetRoute(const std::string& route) const {

        if (routes_.count(route) == 0) {
            return nullptr;
        }

        return &routes_.at(route);
    }

    const Stop* TransportCatalogue::GetStop(const std::string& stop) const {

        if (stops_.count(stop) == 0) {
            return nullptr;
        }

        return &stops_.at(stop);
    }

    const RouteInformation* TransportCatalogue::GetRouteInformation(const std::string& route) const {

        if (routes_.count(route) == 0) {
            return nullptr;
        }

        const Route& finded_route = routes_.at(route);
        return &finded_route.information;

    }

    std::optional<const std::set<std::string>> TransportCatalogue::GetStopInformation(const std::string& stop) const {

        static std::set<std::string> empty_stop_set;

        if (stops_.count(stop) == 0) {
            return std::nullopt;
        }

        const Stop& searched_stop = stops_.at(stop);
        return stops_to_routes_.at(&searched_stop);

    }

    void TransportCatalogue::AddDistanceToOtherStop(const std::pair<const std::string&, const std::string&> from_to_pair, size_t distance) {

        if (stops_.count(from_to_pair.first) == 0 || stops_.count(from_to_pair.second) == 0) {
            return;
        }

        stops_distances_.insert({ std::make_pair(&stops_.at(from_to_pair.first), &stops_.at(from_to_pair.second)), distance });

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
