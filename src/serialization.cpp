#include "serialization.h"

using namespace transport_catalogue;

void SerializeBase(const transport_catalogue::TransportCatalogue& catalogue, 
                   const RouteSettings& route_settings, 
                   const render::MapSettings& map_settings, 
                   std::ostream& output) {

    transport_system::TransportCatalogue catalogue_to_save;

    //Сериализуем транспортный справочник
    SerializeTransportCatalogue(catalogue_to_save, catalogue);

    //Сериализуем настройки маршрутизации
    SerializeRoutingSettings(catalogue_to_save, route_settings);

    //Сериализуем настройки отображения 
    SerializeRenderSettings(catalogue_to_save, map_settings);

    //По этим объектам можно будет целиком восстановить исходную базу
    //поэтому можно приступать к самой сериализации
    catalogue_to_save.SerializeToOstream(&output);

}

void SerializeTransportCatalogue(transport_system::TransportCatalogue& catalogue_to_save, const TransportCatalogue& catalogue) {

    //Сериализуем остановки
    SerializeStops(catalogue_to_save, catalogue);

    //Сериализуем автобусы
    SerializeBuses(catalogue_to_save, catalogue);

    //Сериализуем расстояния между остановками
    SerializeDistancesBetweenStops(catalogue_to_save, catalogue);

}

void SerializeStops(transport_system::TransportCatalogue& catalogue_to_save, const TransportCatalogue& catalogue) {

    const auto& all_stops = catalogue.GetAllStops();
    for (const auto& stop : all_stops) {
        auto* new_stop = catalogue_to_save.add_stop();
        new_stop->set_name(stop.stop_name);
        new_stop->set_latitude(stop.coordinates.lat);
        new_stop->set_longitude(stop.coordinates.lng);
    }

}

void SerializeBuses(transport_system::TransportCatalogue& catalogue_to_save, const TransportCatalogue& catalogue) {
    
    const auto& all_buses = catalogue.GetAllBuses();
    for (const auto& [bus_name, bus_pointer] : all_buses) {
        auto* new_bus = catalogue_to_save.add_bus();
        new_bus->set_name(std::string(bus_name));
        bool is_roundtrip = bus_pointer->type == BusType::Circle ? true : false;
        new_bus->set_is_roundtrip(is_roundtrip);
        for (const auto* stop : bus_pointer->stops) {
            new_bus->add_stop(catalogue.GetStopId(stop->stop_name));
        }
    }

}

void SerializeDistancesBetweenStops(transport_system::TransportCatalogue& catalogue_to_save, const TransportCatalogue& catalogue) {
    
    const auto& distances_list_of_stops = catalogue.GetDistancesListBetweenStops();
    for (const auto& [stops_pair, distance] : distances_list_of_stops) {
        auto* new_distance_between_stops = catalogue_to_save.add_distance_between_stops();
        new_distance_between_stops->set_stop_from(catalogue.GetStopId(stops_pair.first->stop_name));
        new_distance_between_stops->set_stop_to(catalogue.GetStopId(stops_pair.second->stop_name));
        new_distance_between_stops->set_distance(distance);
    }

}

void SerializeRoutingSettings(transport_system::TransportCatalogue& catalogue_to_save, const RouteSettings& route_settings) {

    transport_system::RoutingSettings routing_settings;
    routing_settings.set_bus_wait_time(route_settings.bus_wait_time);
    routing_settings.set_bus_velocity(route_settings.bus_velocity);
    *catalogue_to_save.mutable_routing_settings() = routing_settings;

}

void SerializeRenderSettings(transport_system::TransportCatalogue& catalogue_to_save, const render::MapSettings& source_render_settings) {

    transport_system::RenderSettings render_settings;
    render_settings.set_width(source_render_settings.GetWidth());
    render_settings.set_height(source_render_settings.GetHeight());
    render_settings.set_padding(source_render_settings.GetPadding());
    render_settings.set_line_width(source_render_settings.GetLineWidth());
    render_settings.set_stop_radius(source_render_settings.GetStopRadius());
    render_settings.set_bus_label_font_size(source_render_settings.GetBusLabelFontSize());
    render_settings.set_stop_label_font_size(source_render_settings.GetStopLabelFontSize());
    render_settings.set_underlayer_width(source_render_settings.GetUnderlayerWidth());

    transport_system::LabelOffset bus_label_offset;
    svg::Point source_bus_label_offset = source_render_settings.GetBusLabelOffset();

    bus_label_offset.set_dx(source_bus_label_offset.x);
    bus_label_offset.set_dy(source_bus_label_offset.y);
    *render_settings.mutable_bus_label_offset() = bus_label_offset;
    
    transport_system::LabelOffset stop_label_offset;
    svg::Point source_stop_label_offset = source_render_settings.GetStopLabelOffset();

    stop_label_offset.set_dx(source_stop_label_offset.x);
    stop_label_offset.set_dy(source_stop_label_offset.y);
    *render_settings.mutable_stop_label_offset() = stop_label_offset;

    *render_settings.mutable_underlayer_color() = GetSerializedColor(source_render_settings.GetUnderlayerColor());

    const std::vector<svg::Color>& source_color_palette = source_render_settings.GetColorPalette();

    for (const svg::Color& color : source_color_palette) {
        auto* new_color_palette = render_settings.add_color_palette();
        *new_color_palette = GetSerializedColor(color);
    }

    *catalogue_to_save.mutable_render_settings() = render_settings;
}

transport_system::Color GetSerializedColor(svg::Color color_to_serialize) {

    transport_system::Color serialized_color;

    if (std::holds_alternative<std::string>(color_to_serialize)) {
        serialized_color.set_color_name(std::get<std::string>(color_to_serialize));
    }
    else if (std::holds_alternative<svg::Rgb>(color_to_serialize)) {
        svg::Rgb rgb = std::get<svg::Rgb>(color_to_serialize);
        serialized_color.add_rgb(static_cast<int>(rgb.red));
        serialized_color.add_rgb(static_cast<int>(rgb.green));
        serialized_color.add_rgb(static_cast<int>(rgb.blue));
    }
    else if (std::holds_alternative<svg::Rgba>(color_to_serialize)) {
        svg::Rgba rgba = std::get<svg::Rgba>(color_to_serialize);
        serialized_color.add_rgb(static_cast<int>(rgba.red));
        serialized_color.add_rgb(static_cast<int>(rgba.green));
        serialized_color.add_rgb(static_cast<int>(rgba.blue));
        serialized_color.set_alpha(rgba.opacity);
    }

    return serialized_color;
}

void DeserializeBase(std::istream& input, TransportCatalogue& catalogue, RouteSettings& route_settings, render::MapSettings& map_customizer) {
    transport_system::TransportCatalogue catalogue_to_read;
    catalogue_to_read.ParseFromIstream(&input);

    //Восстановим транспортный справочник
    DeserializeTransportCatalogue(catalogue, catalogue_to_read);

    //Восстановим настройки маршрутизации
    DeserializeRouteSettings(route_settings, catalogue_to_read);

    //Восстановим настройки рисования карты
    DeserializeRenderSettings(map_customizer, catalogue_to_read);

}

void DeserializeTransportCatalogue(TransportCatalogue& catalogue, const transport_system::TransportCatalogue& catalogue_to_read) {

    //Первым делом восстановим все остановки
    DeserializeStops(catalogue, catalogue_to_read);

    //Далее будем восстанавливать автобусы
    DeserializeBuses(catalogue, catalogue_to_read);

    //Зададим расстояния, ранее восстановленным остановкам
    DeserializeDistancesBetweenStops(catalogue, catalogue_to_read);

}

void DeserializeStops(TransportCatalogue& catalogue, const transport_system::TransportCatalogue& catalogue_to_read) {

    for (int i = 0; i < catalogue_to_read.stop_size(); ++i) {
        auto& stop = catalogue_to_read.stop(i);
        catalogue.AddStop(stop.name(), { stop.latitude(), stop.longitude() });
    }

}

void DeserializeBuses(TransportCatalogue& catalogue, const transport_system::TransportCatalogue& catalogue_to_read) {

    for (int i = 0; i < catalogue_to_read.bus_size(); ++i) {
        auto& bus = catalogue_to_read.bus(i);
        std::vector<std::string> stops(bus.stop_size());
        for (int j = 0; j < bus.stop_size(); ++j) {
            stops[j] = catalogue.GetStopNameById(bus.stop(j));
        }
        BusType bus_type = bus.is_roundtrip() == true ? BusType::Circle : BusType::Forward;
        catalogue.AddBus(bus.name(), stops, bus_type);
    }

}

void DeserializeDistancesBetweenStops(TransportCatalogue& catalogue, const transport_system::TransportCatalogue& catalogue_to_read) {

    for (int i = 0; i < catalogue_to_read.distance_between_stops_size(); ++i) {
        auto& distance_between_stops = catalogue_to_read.distance_between_stops(i);
        std::string_view stop_from = catalogue.GetStopNameById(distance_between_stops.stop_from());
        std::string_view stop_to = catalogue.GetStopNameById(distance_between_stops.stop_to());
        catalogue.SetDistanceBetweenStops(stop_from, stop_to, distance_between_stops.distance());
    }

}

void DeserializeRouteSettings(RouteSettings& route_settings, const transport_system::TransportCatalogue& catalogue_to_read) {

    const auto& serialized_routing_settings = catalogue_to_read.routing_settings();
    route_settings.bus_wait_time = serialized_routing_settings.bus_wait_time();
    route_settings.bus_velocity = serialized_routing_settings.bus_velocity();
}

void DeserializeRenderSettings(render::MapSettings& map_customizer, const transport_system::TransportCatalogue& catalogue_to_read) {

    const auto& serialized_map_settings = catalogue_to_read.render_settings();

    map_customizer.SetWidth(serialized_map_settings.width());
    map_customizer.SetHeight(serialized_map_settings.height());
    map_customizer.SetPadding(serialized_map_settings.padding());
    map_customizer.SetLineWidth(serialized_map_settings.line_width());
    map_customizer.SetStopRadius(serialized_map_settings.stop_radius());
    map_customizer.SetBusLabelFontSize(serialized_map_settings.bus_label_font_size());
    map_customizer.SetStopLabelFontSize(serialized_map_settings.stop_label_font_size());
    map_customizer.SetUnderlayerWidth(serialized_map_settings.underlayer_width());

    const auto& serialized_bus_label_offset = serialized_map_settings.bus_label_offset();
    svg::Point bus_label_offset;
    bus_label_offset.x = serialized_bus_label_offset.dx();
    bus_label_offset.y = serialized_bus_label_offset.dy();
    map_customizer.SetBusLabelOffset(bus_label_offset);

    const auto& serialized_stop_label_offset = serialized_map_settings.stop_label_offset();
    svg::Point stop_label_offset;
    stop_label_offset.x = serialized_stop_label_offset.dx();
    stop_label_offset.y = serialized_stop_label_offset.dy();
    map_customizer.SetStopLabelOffset(stop_label_offset);
    map_customizer.SetUnderlayerColor(GetDeserializedColor(serialized_map_settings.underlayer_color()));

    const int serialized_color_palette_size = serialized_map_settings.color_palette_size();
    std::vector<svg::Color> deserialized_color_palette;

    for (int i = 0; i < serialized_color_palette_size; ++i) {
        deserialized_color_palette.push_back(std::move(GetDeserializedColor(serialized_map_settings.color_palette(i))));
    }

    map_customizer.SetColorPalette(deserialized_color_palette);
}

svg::Color GetDeserializedColor(const transport_system::Color& serialized_color) {

    const std::string& serialized_color_name = serialized_color.color_name();
    const auto& serialized_rgb_size = serialized_color.rgb_size();
    const auto& serialized_opacity = serialized_color.alpha();

    if (serialized_color_name != "") {
        return svg::Color(serialized_color_name);
    }
    else if (serialized_rgb_size && serialized_opacity) {
        svg::Rgba rgba;
        rgba.red = static_cast<uint8_t>(serialized_color.rgb(0));
        rgba.green = static_cast<uint8_t>(serialized_color.rgb(1));
        rgba.blue = static_cast<uint8_t>(serialized_color.rgb(2));
        rgba.opacity = serialized_opacity;
        return svg::Color(rgba);
    }
    else if (serialized_rgb_size) {
        svg::Rgb rgb;
        rgb.red = static_cast<uint8_t>(serialized_color.rgb(0));
        rgb.green = static_cast<uint8_t>(serialized_color.rgb(1));
        rgb.blue = static_cast<uint8_t>(serialized_color.rgb(2));
        return svg::Color(rgb);
    }

    return svg::Color();
}
