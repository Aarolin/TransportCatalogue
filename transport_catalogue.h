#pragma once

#include "domain.h"
#include "graph.h"

#include <algorithm>
#include <deque>
#include <cmath>
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
        void AddBus(const std::string& route, const std::vector<std::string>& stops, BusType route_type);

        const Bus* GetBus(const std::string& route) const;
        const Stop* GetStop(const std::string& stop) const;

        std::optional<BusInformation> GetBusInformation(const std::string& route) const;
        std::optional<const std::set<std::string_view>> GetStopInformation(const std::string& stop) const;

        void SetDistanceBetweenStops(const std::string& stop_from, const std::string& stop_to, size_t distance);
        size_t GetDistanceBetweenStops(Stop* stop_from, Stop* stop_to) const;

        const std::map<std::string_view, Bus*>& GetAllBuses() const;

        size_t GetStopsCount() const;
        size_t GetStopId(std::string_view stop) const;
        std::string_view GetStopNameById(size_t vertex_id) const;

        //void FillDirectedWeightedGraph(graph::DirectedWeightedGraph<WayInfo>& graph, RouteSettings route_settings);

    private:

        std::map<std::string_view, Bus*> buses_;
        std::unordered_map<std::string_view, Stop*> stops_;
        std::unordered_map<Stop*, std::set<std::string_view>> stops_to_buses_;
        std::unordered_map<std::pair<Stop*, Stop*>, size_t, StopsHasher> stops_distances_;
        std::unordered_map<std::string_view, size_t> stops_to_indexes_;
        std::unordered_map<std::size_t, std::string_view> indexes_to_stops_;
        std::deque<Stop> stops_list_;
        std::deque<Bus> bus_list_;

        /*struct RouteEdge {

            size_t from;
            size_t to;

            size_t distance;

        };

        template <typename InputIt>
        std::vector<size_t> SplitRoute(const InputIt first, const InputIt last) const;
        std::vector<RouteEdge> BuildBeginEndRouteEdgesList(size_t stops_count, const std::vector<size_t>& bus_interval_distances) const;
        std::vector<RouteEdge> BuildEndBeginRouteEdgesList(size_t stops_count, const std::vector<size_t>& bus_interval_distances) const;*/
    };

    /*template <typename InputIt>
    std::vector<size_t> TransportCatalogue::SplitRoute(const InputIt first, const InputIt last) const {

        std::vector<size_t> result;

        for (InputIt first_it = first, second_it = next(first); second_it != last; first_it++, second_it++) {

            auto from = *first_it;
            auto to = *second_it;

            result.push_back(GetDistanceBetweenStops(from, to));

        }

        return result;

    }*/

}


