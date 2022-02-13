#pragma once

#include "transport_catalogue.h"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace outing_queries {

	std::vector<std::string> GetOutputQueries(std::istream& input);

	void OutQueries(std::istream& input, std::ostream& output, const transport_catalogue::TransportCatalogue& catalogue);

	void OutRouteInformation(std::ostream& output, const std::string& route_name, const transport_catalogue::TransportCatalogue& catalogue);

	void OutStopInformation(std::ostream& output, const std::string& stop_name, const transport_catalogue::TransportCatalogue& catalogue);
}
