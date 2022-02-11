#pragma once

#include "transport_catalogue.h"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace outing_queries {

	std::vector<std::string> GetOutputQueries();

	void OutQueries(const std::vector<std::string>& queries, const transport_catalogue::TransportCatalogue& catalogue);

}
