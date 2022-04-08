#include "request_handler.h"

using namespace transport_catalogue;
using namespace std;
using namespace render;
using namespace reading_queries;

RequestHandler::RequestHandler(const TransportCatalogue& catalogue, MapRenderer& renderer) : catalogue_(catalogue),
																							 map_renderer_(renderer) {

}


void RequestHandler::OutRequests(const json::Dict& map_requests, ostream& output) const {

	Print(MakeJSONResponseToRequest(map_requests), output);

}

json::Document RequestHandler::MakeJSONResponseToRequest(const json::Dict& map_requests) const {

	const json::Array& stat_requests = map_requests.at("stat_requests"s).AsArray();

	json::Array all_responses;
	all_responses.reserve(stat_requests.size());

	for (const json::Node& stat_request : stat_requests) {

		const json::Dict& map_stat_request = stat_request.AsMap();

		const string& type_request = map_stat_request.at("type"s).AsString();
		const int request_id = map_stat_request.at("id"s).AsInt();

		json::Dict response;

		if (type_request == "Map") {

			MakeMapResponse(response);
			response.insert({ "request_id"s, json::Node(request_id) });
			all_responses.push_back(json::Node(move(response)));
			continue;
		}

		const string& request_value = map_stat_request.at("name"s).AsString();
		if (type_request == "Stop"s) {
			
			ProcessResponseToRequest(MakeStopResponse(request_value), response);
		} 
		else {

			ProcessResponseToRequest(MakeBusResponse(request_value), response);
		}

		response.insert({ "request_id"s, json::Node(request_id) });
		all_responses.push_back(json::Node(move(response)));
	}

	json::Document doc_result(json::Node(move(all_responses)));
	return doc_result;
}

void RequestHandler::ProcessResponseToRequest(optional<json::Dict> opt_response_val, json::Dict& result) const {

	if (opt_response_val.has_value()) {
		result = *opt_response_val;
	}
	else {
		InsertErrorToResponse(result);
	}

}

optional<json::Dict> RequestHandler::MakeBusResponse(const string& bus_name) const {

	const auto route_info = catalogue_.GetRouteInformation(bus_name);
	json::Dict result;

	if (route_info.has_value()) {

		const RouteInformation& route = *route_info;
		result.insert({ "curvature"s, json::Node(route.curvature) });
		result.insert({ "route_length"s, json::Node(static_cast<int>(route.route_length)) });
		result.insert({ "stop_count"s, json::Node(static_cast<int>(route.stops_count)) });
		result.insert({ "unique_stop_count"s, json::Node(static_cast<int>(route.unique_stops_count)) });
		return result;

	}

	return nullopt;
}

optional<json::Dict> RequestHandler::MakeStopResponse(const string& stop_name) const {

	const auto buses_by_stop = catalogue_.GetStopInformation(stop_name);

	if (buses_by_stop.has_value()) {

		json::Dict result;
		const auto& buses = *buses_by_stop;
		vector<json::Node> buses_list;

		for (const string_view& bus : buses) {
			buses_list.push_back(json::Node(move(string(bus))));
		}

		result.insert({ "buses", json::Node(move(buses_list)) });

		return result;
	}

	return nullopt;

}

void RequestHandler::InsertErrorToResponse(json::Dict& response) const {

	response.insert({ "error_message", json::Node("not found"s) });

}

void RequestHandler::MakeMapResponse(json::Dict& result) const {

	ostringstream map_output(""s);

	const auto& routes_to_draw = catalogue_.GetAllRoutes();

	map_renderer_.RenderMap(map_output, routes_to_draw);
	result.insert({ "map"s, json::Node(map_output.str()) });
}
