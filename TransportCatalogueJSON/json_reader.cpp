#include "json_reader.h"

namespace reading_queries {

	using namespace std;
	using namespace geo;
	using namespace render;
	using namespace transport_catalogue;

	json::Document ReadQueries(std::istream& is) {
		return json::Load(is);
	}

	void ExecuteFillInRequests(transport_catalogue::TransportCatalogue& catalogue, const json::Dict& requests) {
		
		const json::Node& base_requests_node = requests.at("base_requests"s);

		const json::Array& base_requests = base_requests_node.AsArray();

		vector<const json::Node*> stops_requests;
		vector<const json::Node*> buses_requests;

		SplitFillingRequests(base_requests, stops_requests, buses_requests);

		ExecuteFillStopRequests(catalogue, stops_requests);
		ExecuteFillBusRequests(catalogue, buses_requests);

	}
	
	void SplitFillingRequests(const json::Array& base_requests, vector<const json::Node*>& stops_requests, vector<const json::Node*>& buses_requests) {

		for (const json::Node& request : base_requests) {

			const json::Dict& base_request = request.AsMap();
			const string& request_type = base_request.at("type").AsString();

			if (request_type == "Bus") {
				buses_requests.push_back(&request);
			}
			else {
				stops_requests.push_back(&request);
			}

		}

	}

	void ExecuteFillStopRequests(transport_catalogue::TransportCatalogue& catalogue, vector<const json::Node*>& stop_requests) {

		unordered_map<string, unordered_map<string, int>> additional_stops_requests;

		for (const json::Node* stop_request : stop_requests) {

			try {
				const json::Dict& request = stop_request->AsMap();
				const string& stop_name = request.at("name"s).AsString();
				double latitude = request.at("latitude"s).AsDouble();
				double longitude = request.at("longitude"s).AsDouble();
				catalogue.AddStop(stop_name, Coordinates{ latitude, longitude });

				const json::Dict& road_distance = request.at("road_distances"s).AsMap();

				if (!road_distance.empty()) {

					additional_stops_requests.insert({ stop_name, {} });

					for (const auto& [stop, node_distance] : road_distance) {

						unordered_map<string, int>& distances_to_stop = additional_stops_requests.at(stop_name);
						int distance = node_distance.AsInt();
						distances_to_stop.insert({ stop, distance });

					}
				}

			}
			catch (json::ParsingError& rn_error) {
				cout << rn_error.what() << endl;
			}
			catch (logic_error& lg_error) {
				cout << lg_error.what() << endl;
			}
			catch (exception& std_exp) {
				cout << std_exp.what() << endl;
			}
			catch (...) {
				cout << "Unknown error"sv << endl;
			}

		}
		
		for (const auto& [stop_from, distances] : additional_stops_requests) {

			for (const auto& [stop_to, distance] : distances) {
				catalogue.SetDistanceBetweenStops(stop_from, stop_to, static_cast<size_t>(distance));
			}

		}

	}
	
	void ExecuteFillBusRequests(transport_catalogue::TransportCatalogue& catalogue, vector<const json::Node*>& bus_requests) {

		for (const json::Node* bus_request : bus_requests) {

			try {
				const json::Dict& request = bus_request->AsMap();
				const string& bus_name = request.at("name"s).AsString();
				const json::Array& stops_node = request.at("stops"s).AsArray();
				vector<string> stops_at_route;
				stops_at_route.reserve(stops_node.size());
				
				for (const json::Node& stop_node : stops_node) {

					const string& stop = stop_node.AsString();
					stops_at_route.push_back(stop);
				}

				bool is_roundtrip = request.at("is_roundtrip"s).AsBool();
				
				RouteType route_type = is_roundtrip ? RouteType::Circle : RouteType::Forward;

				catalogue.AddRoute(bus_name, stops_at_route, route_type);

			}
			catch (json::ParsingError& rt_error) {
				cout << rt_error.what() << endl;
			}
			catch (logic_error& lg_error) {
				cout << lg_error.what() << endl;
			} 
			catch (exception& std_except) {
				cout << std_except.what() << endl;
			}
			catch (...) {
				cout << "Unknow error"sv << endl;
			}
		}


	}

	render::MapSettings GetMapCustomizer(const json::Dict& requests) {

		return render::MapSettings(requests.at("render_settings").AsMap());

	}

	JSONRequestBuilder::JSONRequestBuilder(const TransportCatalogue& catalogue, MapRenderer& renderer) 
		: catalogue_(catalogue),
		map_renderer_(renderer) {

	}

	json::Document JSONRequestBuilder::MakeJSONResponseToRequest(const json::Dict& map_requests) {

		const json::Array& stat_requests = map_requests.at("stat_requests"s).AsArray();

		json::Array all_responses;
		all_responses.reserve(stat_requests.size());

		for (const json::Node& stat_request : stat_requests) {

			const json::Dict& map_stat_request = stat_request.AsMap();

			const string& type_request = map_stat_request.at("type"s).AsString();
			const int request_id = map_stat_request.at("id"s).AsInt();

			json::Dict response;

			if (type_request == "Map") {

				response = MakeMapResponse();
			}
			else {

				const string& request_value = map_stat_request.at("name"s).AsString();
				if (type_request == "Stop"s) {

					response = MakeStopResponse(request_value);
				}
				else {

					response = MakeBusResponse(request_value);
				}

			}


			response.insert({ "request_id"s, json::Node{request_id} });
			all_responses.push_back(json::Node{ move(response) });
		}

		json::Document doc_result(json::Node{ move(all_responses) });
		return doc_result;
	}

	json::Dict JSONRequestBuilder::MakeBusResponse(const string& bus_name) const {

		const auto route_info = catalogue_.GetRouteInformation(bus_name);
		json::Dict result;

		if (route_info.has_value()) {

			const RouteInformation& route = *route_info;
			result.insert({ "curvature"s, json::Node{route.curvature} });
			result.insert({ "route_length"s, json::Node{static_cast<int>(route.route_length)} });
			result.insert({ "stop_count"s, json::Node{static_cast<int>(route.stops_count)} });
			result.insert({ "unique_stop_count"s, json::Node{static_cast<int>(route.unique_stops_count)} });
			return result;

		}

		InsertErrorToResponse(result);
		return result;
	}

	json::Dict JSONRequestBuilder::MakeStopResponse(const string& stop_name) const {

		const auto buses_by_stop = catalogue_.GetStopInformation(stop_name);
		json::Dict result;

		if (buses_by_stop.has_value()) {

			const auto& buses = *buses_by_stop;
			vector<json::Node> buses_list;

			for (const string_view& bus : buses) {
				buses_list.push_back(json::Node{ move(string(bus)) });
			}

			result.insert({ "buses", json::Node{move(buses_list)} });

			return result;
		}

		InsertErrorToResponse(result);
		return result;

	}

	void JSONRequestBuilder::InsertErrorToResponse(json::Dict& response) const {

		response.insert({ "error_message", json::Node{"not found"s} });

	}

	json::Dict JSONRequestBuilder::MakeMapResponse() {

		json::Dict result;
		ostringstream map_output(""s);

		const auto& routes_to_draw = catalogue_.GetAllRoutes();

		map_renderer_.RenderMap(map_output, routes_to_draw);
		result.insert({ "map"s, json::Node{map_output.str()} });
		return result;
	}

}