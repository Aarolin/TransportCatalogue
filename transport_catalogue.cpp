#include "transport_catalogue.h"

namespace transport_catalogue {

    using namespace geo;
    using namespace graph;

    void TransportCatalogue::AddStop(const std::string& stop, Coordinates coordinates) {

        stops_list_.push_back({ stop, std::move(coordinates) });
        auto& last_stop = stops_list_.back();
        stops_.insert({ last_stop.stop_name, &last_stop });
        stops_to_buses_.insert({ &last_stop, {} });
        stops_to_indexes_.insert({ last_stop.stop_name, stops_to_indexes_.size() });
        indexes_to_stops_.insert({ stops_to_indexes_.size() - 1, last_stop.stop_name });
    }

    void TransportCatalogue::AddBus(const std::string& bus, const std::vector<std::string>& stops, BusType route_type) {

        std::vector<Stop*> bus_stops;

        for (const std::string& stop : stops) {
            bus_stops.push_back(stops_.at(stop));
        }

        bus_list_.push_back({ bus, route_type, std::move(bus_stops) });
        auto& last_bus = bus_list_.back();
        buses_.insert({ last_bus.bus_name, &last_bus });

        for (auto stop : last_bus.stops) {
            stops_to_buses_.at(stop).insert(last_bus.bus_name);
        }

    }

    const Bus* TransportCatalogue::GetBus(const std::string& bus) const {

        if (buses_.count(bus) == 0) {
            return nullptr;
        }

        return static_cast<const Bus*>(buses_.at(bus));
    }

    const Stop* TransportCatalogue::GetStop(const std::string& stop) const {

        if (stops_.count(stop) == 0) {
            return nullptr;
        }

        return static_cast<const Stop*>(stops_.at(stop));
    }

    std::optional<BusInformation> TransportCatalogue::GetBusInformation(const std::string& route) const {

        if (buses_.count(route) == 0) {
            return std::nullopt;
        }

        Bus* finded_route = buses_.at(route);
        size_t stops_count = finded_route->stops.size();

        if (stops_count == 0) {
            return BusInformation{ stops_count, 0, 0, 0.0 };
        }


        const std::vector<Stop*> route_stops = finded_route->stops;

        size_t route_length = 0;
        double geographical_distance = 0.0;

        for (size_t i = 0; i < stops_count - 1; ++i) {

            geographical_distance += ComputeDistance(route_stops[i]->coordinates, route_stops[i + 1]->coordinates);
            route_length += GetDistanceBetweenStops(route_stops[i], route_stops[i + 1]);

        }

        if (finded_route->type == BusType::Forward) {

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


        return  BusInformation{ stops_count, unique_stops_count, route_length, curvature };

    }

    std::optional<const std::set<std::string_view>> TransportCatalogue::GetStopInformation(const std::string& stop) const {

        if (stops_.count(stop) == 0) {
            return std::nullopt;
        }

        Stop* searched_stop = stops_.at(stop);
        return stops_to_buses_.at(searched_stop);

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

    const std::map<std::string_view, Bus*>& TransportCatalogue::GetAllBuses() const {
        return buses_;
    }

    size_t TransportCatalogue::GetStopsCount() const {
        return stops_list_.size();
    }

   /* void TransportCatalogue::FillDirectedWeightedGraph(graph::DirectedWeightedGraph<WayInfo>& graph, RouteSettings route_settings) {

        double speed_for_calc = static_cast<double>(route_settings.bus_velocity) * 1000;
        speed_for_calc /= static_cast<double>(60);

        for (const auto& [name, ptr] : buses_) {

            std::vector<size_t> bus_interval_distances = SplitRoute(ptr->stops.begin(), ptr->stops.end());
            std::vector<TransportCatalogue::RouteEdge> route_edges = BuildBeginEndRouteEdgesList(ptr->stops.size(), bus_interval_distances);
            
            for (const auto& route_edge : route_edges) {

                graph::Edge<WayInfo> graph_edge;
                graph_edge.from = GetStopId(ptr->stops[route_edge.from]->stop_name);
                graph_edge.to = GetStopId(ptr->stops[route_edge.to]->stop_name);
                
                double road_weight = (static_cast<double>(route_edge.distance) / speed_for_calc) + static_cast<double>(route_settings.bus_wait_time);
                graph_edge.weight = { name, std::abs(static_cast<int>(route_edge.from) - static_cast<int>(route_edge.to)) , road_weight};
                graph.AddEdge(graph_edge);
            }

            if (ptr->type == BusType::Forward) {

                std::vector<size_t> reverse_bus_interval_distances = SplitRoute(ptr->stops.rbegin(), ptr->stops.rend());
                std::vector<TransportCatalogue::RouteEdge> reverse_route_edges = BuildEndBeginRouteEdgesList(ptr->stops.size(), reverse_bus_interval_distances);

                for (const auto& reverse_route_edge : reverse_route_edges) {

                    graph::Edge<WayInfo> graph_edge;
                    graph_edge.from = GetStopId(ptr->stops[reverse_route_edge.from]->stop_name);
                    graph_edge.to = GetStopId(ptr->stops[reverse_route_edge.to]->stop_name);
                    double road_weight = (static_cast<double>(reverse_route_edge.distance) / speed_for_calc) + static_cast<double>(route_settings.bus_wait_time);

                    graph_edge.weight = {name, std::abs(static_cast<int>(reverse_route_edge.from) - static_cast<int>(reverse_route_edge.to)), road_weight};
                    graph.AddEdge(graph_edge);
                }

            }

        }

    }*/

    size_t TransportCatalogue::GetStopId(std::string_view stop) const {
        return stops_to_indexes_.at(stop);
    }

    std::string_view TransportCatalogue::GetStopNameById(size_t vertex_id) const {
        return indexes_to_stops_.at(vertex_id);
    }

    

    /*std::vector<TransportCatalogue::RouteEdge> TransportCatalogue::BuildBeginEndRouteEdgesList(size_t stops_count, const std::vector<size_t>& bus_interval_distances) const {

        std::vector<TransportCatalogue::RouteEdge> result;
        
        for (size_t i = 0; i < stops_count - 1; ++i) {

            for (size_t j = i + 1; j < stops_count; ++j) {

                size_t distance = std::accumulate(bus_interval_distances.begin() + i, bus_interval_distances.begin() + j, 0);
                result.push_back({i, j, distance});

            }

        }

        return result;
    }

    std::vector<TransportCatalogue::RouteEdge> TransportCatalogue::BuildEndBeginRouteEdgesList(size_t stops_count, const std::vector<size_t>& bus_interval_distances) const {

        std::vector<TransportCatalogue::RouteEdge> result;

        for (int from = stops_count - 1, i = 0; from > -1; --from, ++i) {

            for (int to = from - 1, j = i + 1; to > -1; --to, ++j) {

                size_t distance = std::accumulate(bus_interval_distances.begin() + i, bus_interval_distances.begin() + j, 0);
                result.push_back({ static_cast<size_t>(from), static_cast<size_t>(to), distance });

            }

        }

        return result;

    }*/


} // namespace trasport_catalogue


