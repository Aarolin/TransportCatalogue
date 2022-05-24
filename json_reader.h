#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

#include <sstream>

namespace reading_queries {

	json::Document ReadQueries(std::istream& is);

	void ExecuteFillInRequests(transport_catalogue::TransportCatalogue& catalogue, const json::Dict& requests);
	
	void ExecuteFillStopRequests(transport_catalogue::TransportCatalogue& catalogue, std::vector<const json::Node*>& stop_requests);
	
	void ExecuteFillBusRequests(transport_catalogue::TransportCatalogue& catalogue, std::vector<const json::Node*>& bus_requests);
	
	void SplitFillingRequests(const json::Array& base_requests,
									std::vector<const json::Node*>& stops_requests,
									std::vector<const json::Node*>& buses_requests);

	render::MapSettings GetMapCustomizer(const json::Dict& requests);
	
	class JSONRequestBuilder {

	public:

		JSONRequestBuilder(const transport_catalogue::TransportCatalogue& catalogue, render::MapRenderer& renderer);

		json::Document MakeJSONResponseToRequest(const json::Dict& map_requests);


	private:

		json::Dict MakeBusResponse(const std::string& bus_name) const;
		json::Dict MakeStopResponse(const std::string& stop_name) const;
		json::Dict MakeMapResponse();

		void InsertErrorToResponse(json::Dict& response) const;

		const transport_catalogue::TransportCatalogue& catalogue_;
		render::MapRenderer& map_renderer_;

	};
}