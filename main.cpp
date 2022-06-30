#include "json_reader.h"
#include "request_handler.h"
#include "graph.h"
#include "router.h"

#include <iostream>

int main() {

	using namespace transport_catalogue;
	using namespace reading_queries;
	using namespace json;
	using namespace graph;

	Document queries = ReadQueries(std::cin);
	TransportCatalogue catalogue;

	const Dict& queries_map = queries.GetRoot().AsDict();

	ExecuteFillInRequests(catalogue, queries_map);

	render::MapSettings customizer = GetMapCustomizer(queries_map);
	render::MapProjector projector(customizer.GetWidth(), customizer.GetHeight(), customizer.GetPadding());
	render::MapRenderer map_renderer(customizer, projector);

	DirectedWeightedGraph<double> routes_graph(catalogue.ComputeVertexCount());

	catalogue.FillDirectedWeightedGraph(routes_graph, GetRouteSettings(queries_map));
	Router map_router(routes_graph);


	JSONRequestBuilder json_doc_builder(catalogue, map_renderer, map_router, routes_graph);
	RequestHandler handler(json_doc_builder);

	handler.OutRequests(queries_map, std::cout);

	return 0;
}