#pragma once

#include "transport_router.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "router.h"

#include <filesystem>
#include <sstream>

namespace reading_queries {

	json::Document ReadQueries(std::istream& is);

	void ExecuteFillInRequests(transport_catalogue::TransportCatalogue& catalogue, const json::Dict& requests);
	
	void ExecuteFillStopRequests(transport_catalogue::TransportCatalogue& catalogue, std::vector<const json::Node*>& stop_requests);
	
	void ExecuteFillBusRequests(transport_catalogue::TransportCatalogue& catalogue, std::vector<const json::Node*>& bus_requests);
	
	void SplitFillingRequests(const json::Array& base_requests,
									std::vector<const json::Node*>& stops_requests,
									std::vector<const json::Node*>& buses_requests);

	std::filesystem::path GetSerializeSettingsPath(const json::Dict& requests);

	render::MapSettings GetMapCustomizer(const json::Dict& requests);
	RouteSettings GetRouteSettings(const json::Dict& requests);
	
	class JSONRequestBuilder {

	public:
        
		JSONRequestBuilder(const transport_catalogue::TransportCatalogue& catalogue, 
			render::MapRenderer& renderer, const TransportRouter& transport_router,
			const RouteSettings& route_settings_);

		json::Document MakeJSONResponseToRequest(const json::Dict& map_requests);

	private:

		void MakeBusResponse(const std::string& bus_name, json::Builder& answer_builder) const;
		void MakeStopResponse(const std::string& stop_name, json::Builder& answer_builder) const;
		void MakeMapResponse(json::Builder& answer_builder);
		void MakeRouteRequest(json::Builder& answer_builder, const std::string& route_begin, const std::string& route_end) const;
		void InsertErrorToResponse(json::Builder& answer_builder) const;

		const transport_catalogue::TransportCatalogue& catalogue_;
		render::MapRenderer& map_renderer_;
		const TransportRouter& transport_router_;
		const RouteSettings& route_settings_;

	};

} // namespace reading_queries