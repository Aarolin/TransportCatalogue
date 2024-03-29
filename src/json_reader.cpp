#include "json_reader.h"

namespace reading_queries {

	using namespace std;
	using namespace geo;
	using namespace render;
	using namespace transport_catalogue;
	using namespace graph;

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

			const json::Dict& base_request = request.AsDict();
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
				const json::Dict& request = stop_request->AsDict();
				const string& stop_name = request.at("name"s).AsString();
				double latitude = request.at("latitude"s).AsDouble();
				double longitude = request.at("longitude"s).AsDouble();
				catalogue.AddStop(stop_name, Coordinates{ latitude, longitude });

				const json::Dict& road_distance = request.at("road_distances"s).AsDict();

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
				const json::Dict& request = bus_request->AsDict();
				const string& bus_name = request.at("name"s).AsString();
				const json::Array& stops_node = request.at("stops"s).AsArray();
				vector<string> stops_at_route;
				stops_at_route.reserve(stops_node.size());
				
				for (const json::Node& stop_node : stops_node) {

					const string& stop = stop_node.AsString();
					stops_at_route.push_back(stop);
				}

				bool is_roundtrip = request.at("is_roundtrip"s).AsBool();
				
				BusType route_type = is_roundtrip ? BusType::Circle : BusType::Forward;

				catalogue.AddBus(bus_name, stops_at_route, route_type);

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

	std::filesystem::path GetSerializeSettingsPath(const json::Dict& requests) {
		
		const json::Dict& serialization_settings = requests.at("serialization_settings"s).AsDict();
		std::filesystem::path serialization_settings_path(serialization_settings.at("file"s).AsString());
		return serialization_settings_path;

	}

	render::MapSettings GetMapCustomizer(const json::Dict& requests) {
		return render::MapSettings(requests.at("render_settings").AsDict());
	}

	RouteSettings GetRouteSettings(const json::Dict& requests) {

		const auto& route_settings_dict = requests.at("routing_settings").AsDict();
		int bus_wait_time = route_settings_dict.at("bus_wait_time").AsInt();
		int bus_velocity = route_settings_dict.at("bus_velocity").AsInt();

		return { bus_wait_time, bus_velocity };
	}

	JSONRequestBuilder::JSONRequestBuilder(const TransportCatalogue& catalogue, MapRenderer& renderer,
		const TransportRouter& transport_router, const RouteSettings& route_settings) :
		catalogue_(catalogue),
		map_renderer_(renderer),
		transport_router_(transport_router),
		route_settings_(route_settings) {

	}


	json::Document JSONRequestBuilder::MakeJSONResponseToRequest(const json::Dict& map_requests) {

		const json::Array& stat_requests = map_requests.at("stat_requests"s).AsArray();

		json::Builder answer_builder;
		answer_builder.StartArray();

		for (const json::Node& stat_request : stat_requests) {

			const json::Dict& map_stat_request = stat_request.AsDict();

			const string& type_request = map_stat_request.at("type"s).AsString();
			const int request_id = map_stat_request.at("id"s).AsInt();

			answer_builder.StartDict();

			if (type_request == "Map"s) {
				MakeMapResponse(answer_builder);
			}
			else if (type_request == "Route"s) {

				const string& route_begin = map_stat_request.at("from"s).AsString();
				const string& route_end = map_stat_request.at("to"s).AsString();

				MakeRouteRequest(answer_builder, route_begin, route_end);

			}
			else {

				const string& request_value = map_stat_request.at("name"s).AsString();

				if (type_request == "Stop"s) {
					MakeStopResponse(request_value, answer_builder);
				}
				else {
					MakeBusResponse(request_value, answer_builder);
				}

			}

			answer_builder.Key("request_id"s).Value(request_id);
			answer_builder.EndDict();

		}

		answer_builder.EndArray();
		return json::Document{answer_builder.Build()};
	}

	void JSONRequestBuilder::MakeBusResponse(const string& bus_name, json::Builder& answer_builder) const {

		const auto bus_info = catalogue_.GetBusInformation(bus_name);

		if (bus_info.has_value()) {

			const BusInformation& route = *bus_info;
			answer_builder.Key("curvature"s).Value(route.curvature);
			answer_builder.Key("route_length"s).Value(static_cast<int>(route.route_length));
			answer_builder.Key("stop_count"s).Value(static_cast<int>(route.stops_count));
			answer_builder.Key("unique_stop_count"s).Value(static_cast<int>(route.unique_stops_count));
			return;
		}

		InsertErrorToResponse(answer_builder);
	}

	void JSONRequestBuilder::MakeStopResponse(const string& stop_name, json::Builder& answer_builder) const {

		const auto buses_by_stop = catalogue_.GetStopInformation(stop_name);

		if (buses_by_stop.has_value()) {

			const auto& buses = *buses_by_stop;
			vector<json::Node> buses_list;

			for (const string_view& bus : buses) {
				buses_list.push_back(json::Node{ move(string(bus)) });
			}

			answer_builder.Key("buses"s).Value(move(buses_list));
			return;
		}

		InsertErrorToResponse(answer_builder);

	}

	void JSONRequestBuilder::InsertErrorToResponse(json::Builder& answer_builder) const {

		answer_builder.Key("error_message"s).Value("not found"s);
	}

	void JSONRequestBuilder::MakeMapResponse(json::Builder& answer_builder) {

		ostringstream map_output(""s);

		const auto& routes_to_draw = catalogue_.GetAllBuses();

		map_renderer_.RenderMap(map_output, routes_to_draw);
		answer_builder.Key("map"s).Value(map_output.str());
	}

	void JSONRequestBuilder::MakeRouteRequest(json::Builder& answer_builder, const std::string& route_begin, const std::string& route_end) const {

		std::optional<size_t> vertex_ind_route_begin = catalogue_.GetStopId(route_begin);
		std::optional<size_t> vertex_ind_route_end = catalogue_.GetStopId(route_end);

		if (!vertex_ind_route_begin || !vertex_ind_route_end) {

			InsertErrorToResponse(answer_builder);
			return;

		}
		const auto& graph = transport_router_.GetGraph();
		const auto& router = transport_router_.GetRouter();

		auto route_info = router.BuildRoute(*vertex_ind_route_begin, *vertex_ind_route_end);

		if (!route_info) {

			InsertErrorToResponse(answer_builder);
			return;

		}

		answer_builder.Key("items"s).StartArray();
		const std::vector<EdgeId>& route_edges = route_info->edges;

		double total_time = 0.0;

		for (EdgeId edge_id : route_edges) {

			const auto& edge = graph.GetEdge(edge_id);
			auto stop_name_from = catalogue_.GetStopNameById(edge.from);

			answer_builder.StartDict();
			answer_builder.Key("stop_name"s).Value(std::string(stop_name_from));
			answer_builder.Key("time"s).Value(route_settings_.bus_wait_time);
			answer_builder.Key("type"s).Value("Wait"s);
			answer_builder.EndDict();

			answer_builder.StartDict();
			answer_builder.Key("bus"s).Value(std::string(edge.weight.bus_name));
			answer_builder.Key("span_count"s).Value(edge.weight.stop_count);
			answer_builder.Key("time"s).Value(edge.weight.weight - static_cast<double>(route_settings_.bus_wait_time));
			answer_builder.Key("type"s).Value("Bus"s);
			answer_builder.EndDict();
			total_time += edge.weight.weight;
		}

		answer_builder.EndArray();

		answer_builder.Key("total_time"s).Value(total_time);
	}

} // namespace reading_queries