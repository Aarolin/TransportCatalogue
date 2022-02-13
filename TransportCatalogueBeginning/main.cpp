#include "input_reader.h"
#include "stat_reader.h"

int main() {

    using namespace std;
    using namespace transport_catalogue;
    using namespace reading_queries;
    using namespace outing_queries;


    TransportCatalogue t_catalogue;
    FillTransportCatalogue(t_catalogue, cin);
    OutQueries(cin, cout, t_catalogue);

    return 0;
}