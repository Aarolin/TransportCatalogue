#include "json_reader.h"
#include "request_handler.h"

#include <iostream>

int main() {

	using namespace transport_catalogue;
	using namespace reading_queries;
	using namespace json;

	Document queries = ReadQueries(std::cin);
	TransportCatalogue catalogue;

	const Dict& queries_map = queries.GetRoot().AsDict();

	ExecuteFillInRequests(catalogue, queries_map);

	render::MapSettings customizer = GetMapCustomizer(queries_map);
	render::MapProjector projector(customizer.GetWidth(), customizer.GetHeight(), customizer.GetPadding());
	render::MapRenderer map_renderer(customizer, projector);

	JSONRequestBuilder json_doc_builder(catalogue, map_renderer);

	RequestHandler handler(json_doc_builder);

	handler.OutRequests(queries_map, std::cout);

	return 0;
}