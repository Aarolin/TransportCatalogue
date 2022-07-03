#include "transport_router.h"

TransportRouterBuilder::TransportRouterBuilder(const transport_catalogue::TransportCatalogue& catalogue) :
	catalogue_(catalogue), graph_(catalogue_.GetStopsCount()) {

}

TransportRouterBuilder& TransportRouterBuilder::FillGraph(RouteSettings route_settings) {

	const auto& bus_list = catalogue_.GetAllBuses();
	double speed_for_calc = static_cast<double>(route_settings.bus_velocity) * 1000;
	speed_for_calc /= static_cast<double>(60);

	for (const auto& [name, ptr] : bus_list) {

		std::vector<size_t> bus_interval_distances = SplitRouteIntoIntervals(ptr->stops.begin(), ptr->stops.end());
		std::vector<RouteEdge> route_edges = BuildBeginEndRouteEdgesList(ptr->stops.size(), bus_interval_distances);

		for (const auto& route_edge : route_edges) {

			graph::Edge<WayInfo> graph_edge;
			graph_edge.from = GetVertexIndexByStopName(ptr->stops[route_edge.from]->stop_name);
			graph_edge.to = GetVertexIndexByStopName(ptr->stops[route_edge.to]->stop_name);

			double road_weight = (static_cast<double>(route_edge.distance) / speed_for_calc) + static_cast<double>(route_settings.bus_wait_time);
			graph_edge.weight = { name, std::abs(static_cast<int>(route_edge.from) - static_cast<int>(route_edge.to)) , road_weight };
			graph_.AddEdge(graph_edge);

		}

		if (ptr->type == BusType::Forward) {

			std::vector<size_t> reverse_bus_interval_distances = SplitRouteIntoIntervals(ptr->stops.rbegin(), ptr->stops.rend());
			std::vector<RouteEdge> reverse_route_edges = BuildEndBeginRouteEdgesList(ptr->stops.size(), reverse_bus_interval_distances);

			for (const auto& reverse_route_edge : reverse_route_edges) {

				graph::Edge<WayInfo> graph_edge;
				graph_edge.from = GetVertexIndexByStopName(ptr->stops[reverse_route_edge.from]->stop_name);
				graph_edge.to = GetVertexIndexByStopName(ptr->stops[reverse_route_edge.to]->stop_name);
				double road_weight = (static_cast<double>(reverse_route_edge.distance) / speed_for_calc) + static_cast<double>(route_settings.bus_wait_time);

				graph_edge.weight = { name, std::abs(static_cast<int>(reverse_route_edge.from) - static_cast<int>(reverse_route_edge.to)), road_weight };
				graph_.AddEdge(graph_edge);
			}

		}

	}
	return *this;
}

TransportRouter TransportRouterBuilder::Build() const {
	return { graph_ };
}

std::vector<RouteEdge> TransportRouterBuilder::BuildBeginEndRouteEdgesList(size_t stops_count, const std::vector<size_t>& bus_interval_distances) const {

	std::vector<RouteEdge> result;

	for (size_t i = 0; i < stops_count - 1; ++i) {

		for (size_t j = i + 1; j < stops_count; ++j) {

			size_t distance = std::accumulate(bus_interval_distances.begin() + i, bus_interval_distances.begin() + j, 0);
			result.push_back({ i, j, distance });

		}

	}

	return result;
}

std::vector<RouteEdge> TransportRouterBuilder::BuildEndBeginRouteEdgesList(size_t stops_count, const std::vector<size_t>& bus_interval_distances) const {

	std::vector<RouteEdge> result;

	for (int from = stops_count - 1, i = 0; from > -1; --from, ++i) {

		for (int to = from - 1, j = i + 1; to > -1; --to, ++j) {

			size_t distance = std::accumulate(bus_interval_distances.begin() + i, bus_interval_distances.begin() + j, 0);
			result.push_back({ static_cast<size_t>(from), static_cast<size_t>(to), distance });

		}

	}

	return result;

}


size_t TransportRouterBuilder::GetVertexIndexByStopName(std::string_view stop_name) const {
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
