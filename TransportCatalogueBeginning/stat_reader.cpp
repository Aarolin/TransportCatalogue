#include "stat_reader.h"

namespace outing_queries {

	using namespace transport_catalogue;

	std::vector<std::string> GetOutputQueries() {

		int query_count;
		std::cin >> query_count;
		std::string query;
		std::vector<std::string> output_queries;
		output_queries.reserve(query_count);

		for (int i = 0; i < query_count; ++i) {

			std::getline(std::cin, query);

			if (query == "") {
				std::getline(std::cin, query);
			}

			output_queries.push_back(std::move(query));
			query.clear();
		}

		return output_queries;
	}

	void OutQueries(const std::vector<std::string>& queries, const TransportCatalogue& catalogue) {

		std::cout << std::setprecision(6);
		for (const std::string& query : queries) {

			std::istringstream command(query);
			std::string query_type;
			command >> query_type;

			if (query_type == "Bus") {

				std::string route;
				std::getline(command, route);
				route = route.substr(1);
				const RouteInformation* information = catalogue.GetRouteInformation(route);

				if (information == nullptr) {

					std::cout << "Bus " << route << ": not found" << std::endl;
					continue;

				}

				std::cout << "Bus " << route << ": " << information->stops_count << " stops on route, " << information->unique_stops_count <<
					" unique stops, " << information->route_length << " route length, " << information->curvature << " curvature" << std::endl;
			}

			if (query_type == "Stop") {

				std::string stop;
				std::getline(command, stop);
				stop = stop.substr(1);
				std::optional<const std::set<std::string>> stop_information = catalogue.GetStopInformation(stop);

				if (stop_information == std::nullopt) {

					std::cout << "Stop " << stop << ": not found" << std::endl;
					continue;
				}

				if (stop_information.value().size() == 0) {

					std::cout << "Stop " << stop << ": no buses" << std::endl;
					continue;
				}

				std::cout << "Stop " << stop << ": buses ";

				for (const std::string& bus : stop_information.value()) {
					std::cout << bus << " ";
				}

				std::cout << std::endl;

			}

		}

	}


}
