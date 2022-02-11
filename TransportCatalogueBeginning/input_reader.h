#pragma once

#include "geo.h"
#include "transport_catalogue.h"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace reading_queries {

    enum class QueryType {

        NewRoute,
        NewStop

    };

    struct Queries {

        std::vector<std::string> stop_queries;
        std::vector<std::string> route_queries;

    };

    struct Query {

        QueryType type;
        std::string bus;
        std::vector<std::string> stops;
        transport_catalogue::RouteType route_type;
        std::string stop;
        Coordinates stop_coordinates;
    };

    struct AdditionalQuery {

        std::string from_stop;
        std::vector<std::string> distances_to_stop;

    };

    Queries GetInputQueries();

    void ProcessStopQuery(const std::string& stop_query, transport_catalogue::TransportCatalogue& t_catalogue, std::vector<AdditionalQuery>& add_queries);

    void ProcessRouteQuery(const std::string& route_query, transport_catalogue::TransportCatalogue& t_catalogue);

    void ProcessAdditionalQueries(transport_catalogue::TransportCatalogue& catalogue, const std::vector<AdditionalQuery>& additional_queries);

    void DeleteLastSymbolFromWord(std::string& word);

    transport_catalogue::RouteType DefineRouteType(const std::string& query);


}
