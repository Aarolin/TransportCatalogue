#include "transport_router.h"

TransportRouterBuilder::TransportRouterBuilder(const transport_catalogue::TransportCatalogue& catalogue) :
	catalogue_(catalogue), graph_(catalogue_.GetStopsCount()) {

}

TransportRouterBuilder& TransportRouterBuilder::FillGraph(RouteSettings route_settings) {

	const auto& bus_list = catalogue_.GetAllBuses();
	double bus_speed = static_cast<double>(route_settings.bus_velocity) * 1000;
	bus_speed /= static_cast<double>(60);

	for (const auto& [name, ptr] : bus_list) {

		std::vector<size_t> bus_interval_distances = SplitRouteIntoIntervals(ptr->stops.begin(), ptr->stops.end());
		std::vector<RouteEdge> route_edges = BuildForwardRouteEdgesList(ptr->stops.size(), bus_interval_distances);

		EdgeQuery edge_query{ name, bus_speed, route_settings.bus_wait_time, ptr->stops };
		AddRouteEdgesToGraph(route_edges, edge_query);

		if (ptr->type == BusType::Forward) {

			std::vector<size_t> reverse_bus_interval_distances = SplitRouteIntoIntervals(ptr->stops.rbegin(), ptr->stops.rend());
			std::vector<RouteEdge> reverse_route_edges = BuildBackwardRouteEdgesList(ptr->stops.size(), reverse_bus_interval_distances);

			AddRouteEdgesToGraph(reverse_route_edges, edge_query);
		}

	}
	return *this;
}

TransportRouter TransportRouterBuilder::Build() const {
	return { graph_ };
}

std::vector<RouteEdge> TransportRouterBuilder::BuildForwardRouteEdgesList(size_t stops_count, const std::vector<size_t>& bus_interval_distances) const {

	std::vector<RouteEdge> result;

	for (size_t i = 0; i < stops_count - 1; ++i) {

		for (size_t j = i + 1; j < stops_count; ++j) {

			size_t distance = std::accumulate(bus_interval_distances.begin() + i, bus_interval_distances.begin() + j, 0);
			result.push_back({ i, j, distance });

		}

	}

	return result;
}

std::vector<RouteEdge> TransportRouterBuilder::BuildBackwardRouteEdgesList(size_t stops_count, const std::vector<size_t>& bus_interval_distances) const {

	std::vector<RouteEdge> result;

	for (int from = stops_count - 1, i = 0; from > -1; --from, ++i) {

		for (int to = from - 1, j = i + 1; to > -1; --to, ++j) {

			size_t distance = std::accumulate(bus_interval_distances.begin() + i, bus_interval_distances.begin() + j, 0);
			result.push_back({ static_cast<size_t>(from), static_cast<size_t>(to), distance });

		}

	}

	return result;

}

void TransportRouterBuilder::AddRouteEdgesToGraph(const std::vector<RouteEdge>& route_edges_list, const EdgeQuery& edge_query) {

	const auto& bus_stops_list = edge_query.bus_stops_list;

	for (const auto& route_edge : route_edges_list) {

		graph::Edge<WayInfo> graph_edge;
		graph_edge.from = GetVertexIdByStopName(bus_stops_list[route_edge.from]->stop_name);
		graph_edge.to = GetVertexIdByStopName(bus_stops_list[route_edge.to]->stop_name);

		double road_weight = (static_cast<double>(route_edge.distance) / edge_query.bus_speed) + static_cast<double>(edge_query.bus_wait_time);
		graph_edge.weight = { edge_query.bus_name, std::abs(static_cast<int>(route_edge.from) - static_cast<int>(route_edge.to)) , road_weight };
		graph_.AddEdge(graph_edge);

	}

}


size_t TransportRouterBuilder::GetVertexIdByStopName(std::string_view stop_name) const {
	return catalogue_.GetStopId(stop_name);
}


TransportRouter::TransportRouter(const graph::DirectedWeightedGraph<WayInfo>& graph) :
	graph_(graph), router_(graph_) {

}

const graph::DirectedWeightedGraph<WayInfo>& TransportRouter::GetGraph() const {
	return graph_;
}

const graph::Router<WayInfo>& TransportRouter::GetRouter() const {
	return router_;
}
