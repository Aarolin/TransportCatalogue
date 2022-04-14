#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include <map>
#include <sstream>

class RequestHandler {

public:
	
	RequestHandler(const transport_catalogue::TransportCatalogue& catalogue, render::MapRenderer& renderer);

	void OutRequests(const json::Dict& map_requests, std::ostream& output) const;
	json::Document MakeJSONResponseToRequest(const json::Dict& map_requests) const;


private:

	std::optional<json::Dict> MakeBusResponse(const std::string& bus_name) const;
	std::optional<json::Dict> MakeStopResponse(const std::string& stop_name) const;

	void MakeMapResponse(json::Dict& result) const;

	void ProcessResponseToRequest(std::optional<json::Dict>, json::Dict&) const;
	void InsertErrorToResponse(json::Dict& response) const;


	const transport_catalogue::TransportCatalogue& catalogue_;
	render::MapRenderer& map_renderer_;

};

