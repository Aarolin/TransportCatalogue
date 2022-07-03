#include "json_reader.h"
#include "request_handler.h"
#include "graph.h"
#include "router.h"
#include "transport_router.h"

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

	TransportRouterBuilder transport_router_builder(catalogue);
	transport_router_builder.FillGraph(GetRouteSettings(queries_map));

	TransportRouter transport_router = transport_router_builder.Build();

	JSONRequestBuilder json_doc_builder(catalogue, map_renderer, transport_router);
	RequestHandler handler(json_doc_builder);

	handler.OutRequests(queries_map, std::cout);

	return 0;
}