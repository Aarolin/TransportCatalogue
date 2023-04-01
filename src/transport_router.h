#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

struct RouteEdge {

    size_t from;
    size_t to;
    size_t distance;

};

class TransportRouter;

class TransportRouterBuilder {
public:

    TransportRouterBuilder(const transport_catalogue::TransportCatalogue& catalogue); 
    TransportRouterBuilder& FillGraph(RouteSettings route_settings);
    TransportRouter Build() const;


private:
    const transport_catalogue::TransportCatalogue& catalogue_;
    graph::DirectedWeightedGraph<WayInfo> graph_;

    template <typename InputIt>
    std::vector<size_t> SplitRouteIntoIntervals(const InputIt first, const InputIt last) const;
    std::vector<RouteEdge> BuildForwardRouteEdgesList(size_t stops_count, const std::vector<size_t>& bus_interval_distances) const;
    std::vector<RouteEdge> BuildBackwardRouteEdgesList(size_t stops_count, const std::vector<size_t>& bus_interval_distances) const;

    struct EdgeQuery {

        std::string_view bus_name;
        double bus_speed = 0.0;
        int bus_wait_time = 0;
        const std::vector<Stop*>& bus_stops_list;

    };

    void AddRouteEdgesToGraph(const std::vector<RouteEdge>& route_edges_list, const EdgeQuery& edge_query);
    size_t GetVertexIdByStopName(std::string_view) const;

};

class TransportRouter {
    friend TransportRouterBuilder;
public:

	const graph::DirectedWeightedGraph<WayInfo>& GetGraph() const;
    const graph::Router<WayInfo>& GetRouter() const;

private:

    TransportRouter(const graph::DirectedWeightedGraph<WayInfo>& graph_);
	const graph::DirectedWeightedGraph<WayInfo>& graph_;
    graph::Router<WayInfo> router_;

};


template <typename InputIt>
std::vector<size_t> TransportRouterBuilder::SplitRouteIntoIntervals(const InputIt first, const InputIt last) const {

    std::vector<size_t> result;

    for (InputIt first_it = first, second_it = next(first); second_it != last; first_it++, second_it++) {

        auto from = *first_it;
        auto to = *second_it;

        result.push_back(catalogue_.GetDistanceBetweenStops(from, to));

    }

    return result;

}