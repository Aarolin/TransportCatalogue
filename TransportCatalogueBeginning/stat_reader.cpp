#include "stat_reader.h"

namespace outing_queries {

	using namespace transport_catalogue;

	std::vector<std::string> GetOutputQueries(std::istream& input) {

		int query_count;
		input >> query_count;
		std::string query;
		std::vector<std::string> output_queries;
		output_queries.reserve(query_count);

		for (int i = 0; i < query_count; ++i) {

			std::getline(input, query);

			if (query == "") {
				std::getline(input, query);
			}

			output_queries.push_back(std::move(query));
			query.clear();
		}

		return output_queries;
	}

	void OutQueries(std::istream& input, std::ostream& output, const TransportCatalogue& catalogue) {

		output << std::setprecision(6);

		std::vector<std::string> queries = GetOutputQueries(input);

		for (const std::string& query : queries) {

			std::istringstream command(query);
			std::string query_type;
			command >> query_type;
			std::string object_to_out;
			std::getline(command, object_to_out);
			object_to_out = object_to_out.substr(1);

			if (query_type == "Bus") {

				OutRouteInformation(output, object_to_out, catalogue);
			}
			if (query_type == "Stop") {

				OutStopInformation(output, object_to_out, catalogue);
			}

		}

	}

	void OutRouteInformation(std::ostream& output, const std::string& route_name, const TransportCatalogue& catalogue) {

		std::optional<RouteInformation> information_optional = catalogue.GetRouteInformation(route_name);

		if (information_optional == std::nullopt) {

			output << "Bus " << route_name << ": not found" << std::endl;
			return;
		}

		const RouteInformation& information = information_optional.value();

		output << "Bus " << route_name << ": " << information.stops_count << " stops on route, " << information.unique_stops_count <<
			" unique stops, " << information.route_length << " route length, " << information.curvature << " curvature" << std::endl;

	}

	void OutStopInformation(std::ostream& output, const std::string& stop_name, const TransportCatalogue& catalogue) {

		std::optional<const std::set<std::string_view>> stop_information = catalogue.GetStopInformation(stop_name);

		if (stop_information == std::nullopt) {

			output << "Stop " << stop_name << ": not found" << std::endl;
			return;
		}

		if (stop_information.value().size() == 0) {

			output << "Stop " << stop_name << ": no buses" << std::endl;
			return;
		}

		output << "Stop " << stop_name << ": buses ";

		for (const std::string_view& bus : stop_information.value()) {
			output << bus << " ";
		}

		output << std::endl;

	}
}
