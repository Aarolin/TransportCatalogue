#pragma once

#include "transport_catalogue.pb.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "map_renderer.h"

void SerializeBase(const transport_catalogue::TransportCatalogue& catalogue,
				   const RouteSettings& route_settings,
				   const render::MapSettings& map_settings,
					std::ostream& output);

void SerializeTransportCatalogue(transport_system::TransportCatalogue& catalogue_to_save, const transport_catalogue::TransportCatalogue& catalogue);
void SerializeStops(transport_system::TransportCatalogue& catalogue_to_save, const transport_catalogue::TransportCatalogue& catalogue);
void SerializeBuses(transport_system::TransportCatalogue& catalogue_to_save, const transport_catalogue::TransportCatalogue& catalogue);
void SerializeDistancesBetweenStops(transport_system::TransportCatalogue& catalogue_to_save, const transport_catalogue::TransportCatalogue& catalogue);
void SerializeRoutingSettings(transport_system::TransportCatalogue& catalogue_to_save, const RouteSettings& route_settings);
void SerializeRenderSettings(transport_system::TransportCatalogue& catalogue_to_save, const render::MapSettings& route_settings);

transport_system::Color GetSerializedColor(svg::Color color_to_serialize);

void DeserializeBase(std::istream& input, transport_catalogue::TransportCatalogue& catalogue, RouteSettings& route_settings, render::MapSettings& map_customizer);
void DeserializeTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue, const transport_system::TransportCatalogue& catalogue_to_read);
void DeserializeStops(transport_catalogue::TransportCatalogue& catalogue, const transport_system::TransportCatalogue& catalogue_to_read);
void DeserializeBuses(transport_catalogue::TransportCatalogue& catalogue, const transport_system::TransportCatalogue& catalogue_to_read);
void DeserializeDistancesBetweenStops(transport_catalogue::TransportCatalogue& catalogue, const transport_system::TransportCatalogue& catalogue_to_read);
void DeserializeRouteSettings(RouteSettings& route_settings, const transport_system::TransportCatalogue& catalogue_to_read);
void DeserializeRenderSettings(render::MapSettings& map_customizer, const transport_system::TransportCatalogue& catalogue_to_read);
svg::Color GetDeserializedColor(const transport_system::Color& serialized_color);