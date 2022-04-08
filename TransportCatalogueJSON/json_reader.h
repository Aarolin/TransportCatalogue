#pragma once

#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <cassert>

namespace reading_queries {

	json::Document ReadQueries(std::istream& is);

	void ExecuteFillInRequests(transport_catalogue::TransportCatalogue& catalogue, const json::Dict& requests);
	
	void ExecuteFillStopRequests(transport_catalogue::TransportCatalogue& catalogue, std::vector<const json::Node*>& stop_requests);
	
	void ExecuteFillBusRequests(transport_catalogue::TransportCatalogue& catalogue, std::vector<const json::Node*>& bus_requests);
	
	void SplitFillingRequests(const json::Array& base_requests,
									std::vector<const json::Node*>& stops_requests,
									std::vector<const json::Node*>& buses_requests);

	render::MapSettings GetMapCustomizer(const json::Dict& requests);
	
}