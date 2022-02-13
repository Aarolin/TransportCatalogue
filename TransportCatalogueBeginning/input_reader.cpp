#include "input_reader.h"

namespace reading_queries {

    using namespace transport_catalogue;

    void FillTransportCatalogue(transport_catalogue::TransportCatalogue& t_catalogue, std::istream& input) {

        Queries queries = GetInputQueries(input);
        std::vector<AdditionalQuery> add_queries;

        for (const std::string& stop_query : queries.stop_queries) {

            ProcessStopQuery(stop_query, t_catalogue, add_queries);

        }

        ProcessAdditionalQueries(t_catalogue, add_queries);


        for (const std::string& route_query : queries.route_queries) {

            ProcessRouteQuery(route_query, t_catalogue);
        }
    }

    Queries GetInputQueries(std::istream& input) {
        int queries_count;
        input >> std::setprecision(6);
        input >> queries_count;
        Queries result;

        for (int i = 0; i < queries_count; ++i) {

            std::string query;
            getline(input, query);

            if (query == "") {
                getline(input, query);
            }

            std::istringstream command(query);
            std::string query_type;
            command >> query_type;
            if (query_type == "Bus") {
                result.route_queries.push_back(query.substr(query_type.size() + 1));
            }
            else {
                result.stop_queries.push_back(query.substr(query_type.size() + 1));
            }

        }

        return result;
    }

    void ProcessStopQuery(const std::string& stop_query, TransportCatalogue& t_catalogue, std::vector<AdditionalQuery>& add_queries) {

        std::istringstream command(stop_query);
        Query q;
        std::getline(command, q.stop, ':');
        double lat, lng;
        std::string comma;
        command >> lat;
        command >> comma;
        command >> lng;
        command >> comma;

        std::string distance_to_other_stops;
        std::vector<std::string> distances_to_stop;

        while (std::getline(command, distance_to_other_stops, ',')) {

            distances_to_stop.push_back(move(distance_to_other_stops));
            distance_to_other_stops.clear();

        }

        t_catalogue.AddStop(q.stop, { lat, lng });
        add_queries.push_back({ std::move(q.stop), std::move(distances_to_stop) });
    }

    void ProcessRouteQuery(const std::string& route_query, transport_catalogue::TransportCatalogue& t_catalogue) {

        std::istringstream command(route_query);
        Query q;
        std::getline(command, q.bus, ':');
        std::string stop;
        q.route_type = DefineRouteType(route_query);
        char separator = q.route_type == RouteType::Circle ? '>' : '-';

        while (std::getline(command, stop, separator)) {

            stop = stop.substr(1);

            if (stop.back() == ' ') {
                DeleteLastSymbolFromWord(stop);
            }
            q.stops.push_back(std::move(stop));
            stop.clear();
        }

        t_catalogue.AddRoute(q.bus, q.stops, q.route_type);

    }

    void ProcessAdditionalQueries(TransportCatalogue& catalogue, const std::vector<AdditionalQuery>& additional_queries) {

        for (auto& query : additional_queries) {

            for (const std::string& distance_to_stop : query.distances_to_stop) {

                std::istringstream command(distance_to_stop);
                size_t distance;
                command >> distance;
                std::string to;
                command >> to;
                command >> to;
                std::string stop_to;
                std::getline(command, stop_to);

                stop_to = stop_to.substr(1);

                catalogue.SetDistanceBetweenStops(query.from_stop, stop_to, distance);

            }

        }

    }

    void DeleteLastSymbolFromWord(std::string& word) {
        word = word.substr(0, word.size() - 1);
    }

    RouteType DefineRouteType(const std::string& query) {

        auto circle_symbol = std::find(query.begin(), query.end(), '>');

        if (circle_symbol != query.end()) {
            return RouteType::Circle;
        }

        return RouteType::Forward;

    }
}
