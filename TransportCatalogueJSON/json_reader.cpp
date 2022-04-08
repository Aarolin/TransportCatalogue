#include "json_reader.h"

namespace reading_queries {

	using namespace std;
	using namespace geo;
	using namespace render;

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
}