#include "request_handler.h"
#include "json_reader.h"
#include "serialization.h"

#include <fstream>
#include <iostream>
#include <string_view>
#include <cassert>

using namespace std::literals;

void TestSerializeRenderSettings(std::filesystem::path& path, const render::MapSettings& source_render_customizer) {

    {
        std::ofstream output(path);
        transport_system::TransportCatalogue catalogue_to_serialize;
        SerializeRenderSettings(catalogue_to_serialize, source_render_customizer);
        catalogue_to_serialize.SerializeToOstream(&output);
    }
    
    {
        std::ifstream input(path);
        transport_system::TransportCatalogue catalogue_to_deserialize;
        catalogue_to_deserialize.ParseFromIstream(&input);
        render::MapSettings render_customizer;
        DeserializeRenderSettings(render_customizer, catalogue_to_deserialize);
        assert(render_customizer.GetWidth() == source_render_customizer.GetWidth());
        assert(render_customizer.GetHeight() == source_render_customizer.GetHeight());
        assert(render_customizer.GetPadding() == source_render_customizer.GetPadding());
        assert(render_customizer.GetLineWidth() == source_render_customizer.GetLineWidth());
        assert(render_customizer.GetStopRadius() == source_render_customizer.GetStopRadius());
        assert(render_customizer.GetBusLabelFontSize() == source_render_customizer.GetBusLabelFontSize());
        assert(render_customizer.GetStopLabelFontSize() == source_render_customizer.GetStopLabelFontSize());
        assert(render_customizer.GetUnderlayerWidth() == source_render_customizer.GetUnderlayerWidth());
        assert(render_customizer.GetBusLabelOffset() == source_render_customizer.GetBusLabelOffset());
        assert(render_customizer.GetStopLabelOffset() == source_render_customizer.GetStopLabelOffset());
        assert(render_customizer.GetUnderlayerColor() == source_render_customizer.GetUnderlayerColor());
        assert(render_customizer.GetColorPalette() == source_render_customizer.GetColorPalette());

    } 

}


void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {

    using namespace transport_catalogue;
    using namespace reading_queries;
    using namespace json;
    using namespace graph;

    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        Document queries = ReadQueries(std::cin);
        TransportCatalogue catalogue;

        const Dict& queries_map = queries.GetRoot().AsDict();

        ExecuteFillInRequests(catalogue, queries_map);

        render::MapSettings customizer = GetMapCustomizer(queries_map);
        RouteSettings route_settings = GetRouteSettings(queries_map);

        std::filesystem::path path_to_save_catalogue = GetSerializeSettingsPath(queries_map);
        std::ofstream output(path_to_save_catalogue, std::ios::binary);

        SerializeBase(catalogue, route_settings, customizer, output);

    }
    else if (mode == "process_requests"sv) {

        Document queries = ReadQueries(std::cin);
        const Dict& queries_map = queries.GetRoot().AsDict();
        std::filesystem::path path_to_build_catalogue = GetSerializeSettingsPath(queries_map);
        std::ifstream input(path_to_build_catalogue, std::ios::binary);

        TransportCatalogue catalogue;
        render::MapSettings map_customizer;
        RouteSettings route_settings;

        DeserializeBase(input, catalogue, route_settings, map_customizer);
        render::MapProjector projector(map_customizer.GetWidth(), map_customizer.GetHeight(), map_customizer.GetPadding());
        render::MapRenderer map_renderer(map_customizer, projector);

        TransportRouterBuilder transport_router_builder(catalogue);
        transport_router_builder.FillGraph(route_settings);

        TransportRouter transport_router = transport_router_builder.Build();

        JSONRequestBuilder json_doc_builder(catalogue, map_renderer, transport_router, route_settings);
        RequestHandler handler(json_doc_builder);

        handler.OutRequests(queries_map, std::cout);

    }
    else {
        PrintUsage();
        return 1;
    }
}