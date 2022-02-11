#include "input_reader.h"
#include "stat_reader.h"

int main() {

    using namespace std;
    using namespace transport_catalogue;
    using namespace reading_queries;
    using namespace outing_queries;


    TransportCatalogue t_catalogue;
    Queries queries = GetInputQueries();
    vector<AdditionalQuery> add_queries;

    for (const string& stop_query : queries.stop_queries) {

        ProcessStopQuery(stop_query, t_catalogue, add_queries);

    }

    ProcessAdditionalQueries(t_catalogue, add_queries);


    for (const string& route_query : queries.route_queries) {

        ProcessRouteQuery(route_query, t_catalogue);
    }

    vector<string> output_queries = GetOutputQueries();

    OutQueries(output_queries, t_catalogue);

    return 0;
}