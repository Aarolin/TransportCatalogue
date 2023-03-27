#include "map_renderer.h"

namespace render {

	using namespace std;

	MapSettings::MapSettings(const json::Dict& settings) {

		SetWidth(settings.at("width"s).AsDouble());
		SetHeight(settings.at("height"s).AsDouble());
		SetPadding(settings.at("padding"s).AsDouble());
		SetLineWidth(settings.at("line_width"s).AsDouble());
		SetStopRadius(settings.at("stop_radius"s).AsDouble());
		SetBusLabelFontSize(settings.at("bus_label_font_size"s).AsInt());
		SetBusLabelOffset(settings.at("bus_label_offset"s).AsArray());
		SetStopLabelFontSize(settings.at("stop_label_font_size"s).AsInt());
		SetStopLabelOffset(settings.at("stop_label_offset"s).AsArray());
		SetUnderlayerColor(settings.at("underlayer_color"s));
		SetUnderlayerWidth(settings.at("underlayer_width"s).AsDouble());
		SetColorPalette(settings.at("color_palette"s).AsArray());
	}


	void MapSettings::SetWidth(double width) {
		width_ = width;
	}

	double MapSettings::GetWidth() const {
		return width_;
	}

	void MapSettings::SetHeight(double height) {
		height_ = height;
	}

	double MapSettings::GetHeight() const {
		return height_;
	}

	void MapSettings::SetPadding(double padding) {
		padding_ = padding;
	}

	double MapSettings::GetPadding() const {
		return padding_;
	}

	void MapSettings::SetLineWidth(double line_width) {
		line_width_ = line_width;
	}

	double MapSettings::GetLineWidth() const {
		return line_width_;
	}

	void MapSettings::SetStopRadius(double stop_radius) {
		stop_radius_ = stop_radius;
	}

	double MapSettings::GetStopRadius() const {
		return stop_radius_;
	}

	void MapSettings::SetBusLabelFontSize(int bus_label_font_size) {
		bus_label_font_size_ = bus_label_font_size;
	}

	int MapSettings::GetBusLabelFontSize() const {
		return bus_label_font_size_;
	}

	void MapSettings::SetBusLabelOffset(const json::Array& bus_label_offset) {
		bus_label_offset_.x = bus_label_offset[0].AsDouble();
		bus_label_offset_.y = bus_label_offset[1].AsDouble();
	}

	void MapSettings::SetBusLabelOffset(svg::Point p) {
		bus_label_offset_ = std::move(p);
	}

	svg::Point MapSettings::GetBusLabelOffset() const {
		return bus_label_offset_;
	}

	void MapSettings::SetStopLabelFontSize(int stop_label_font_size) {
		stop_label_font_size_ = stop_label_font_size;
	}

	int MapSettings::GetStopLabelFontSize() const {
		return stop_label_font_size_;
	}

	void MapSettings::SetStopLabelOffset(const json::Array& stop_label_offset) {
		stop_label_offset_.x = stop_label_offset[0].AsDouble();
		stop_label_offset_.y = stop_label_offset[1].AsDouble();
	}

	void MapSettings::SetStopLabelOffset(svg::Point p) {
		stop_label_offset_ = std::move(p);
	}

	svg::Point MapSettings::GetStopLabelOffset() const {
		return stop_label_offset_;
	}

	void MapSettings::SetUnderlayerColor(const json::Node& underlayer_color) {
		underlayer_color_ = std::move(GetColorByNode(underlayer_color));
	}

	void MapSettings::SetUnderlayerColor(svg::Color color) {
		underlayer_color_ = std::move(color);
	}

	const svg::Color& MapSettings::GetUnderlayerColor() const {
		return underlayer_color_;
	}

	void MapSettings::SetUnderlayerWidth(double underlayer_width) {
		underlayer_width_ = underlayer_width;
	}

	double MapSettings::GetUnderlayerWidth() const {
		return underlayer_width_;
	}

	void MapSettings::SetColorPalette(const json::Array& color_palette) {

		for (const json::Node& color : color_palette) {
			color_palette_.push_back(std::move(GetColorByNode(color)));
		}

	}

	void MapSettings::SetColorPalette(std::vector<svg::Color> color_palette) {
		color_palette_ = std::move(color_palette);
	}

	const std::vector<svg::Color>& MapSettings::GetColorPalette() const {
		return color_palette_;
	}

	svg::Color MapSettings::GetColorByNode(const json::Node& node_color) const {

		svg::Color result_color;

		try {

			const std::string& color_name = node_color.AsString();
			result_color = color_name;
			return result_color;

		}
		catch (...) {

			const json::Array& color_description = node_color.AsArray();

			size_t color_description_size = color_description.size();
			int red = color_description[0].AsInt();
			int green = color_description[1].AsInt();
			int blue = color_description[2].AsInt();

			if (color_description_size == 3) {
				result_color = std::move(svg::Rgb(red, green, blue));
			}
			else {
				double opacity = color_description[3].AsDouble();
				result_color = std::move(svg::Rgba(red, green, blue, opacity));
			}

			return result_color;
		}


	}

	MapProjector::MapProjector(double width, double height, double padding) :
		width_(width), height_(height), padding_(padding) {


	}

	double MapProjector::GetWidth() const {
		return width_;
	}

	double MapProjector::GetHeight() const {
		return height_;
	}

	void MapProjector::CalcCoeficients(const deque<const Stop*>& all_stops) {

		const auto min_max_long = CalcMinMaxLong(all_stops);
		const auto min_max_lat = CalcMinMaxLat(all_stops);

		double min_long = min_max_long.first;
		double max_long = min_max_long.second;

		double min_lat = min_max_lat.first;
		double max_lat = min_max_lat.second;

		SetMinLon(min_long);
		SetMaxLat(max_lat);

		double width_zoom_coef = CalcScalCoordinateCoef(min_long, max_long, GetWidth());
		double height_zoom_coef = CalcScalCoordinateCoef(min_lat, max_lat, GetHeight());

		SetZoomCoef(width_zoom_coef, height_zoom_coef);

	}


	pair<double, double> MapProjector::CalcMinMaxLong(const deque<const Stop*>& all_stops) const {

		const auto [min_lon, max_lon] = minmax_element(all_stops.begin(), all_stops.end(), [](const Stop* lhs, const Stop* rhs) {
			return lhs->coordinates.lng < rhs->coordinates.lng;
			});

		return { (*min_lon)->coordinates.lng, (*max_lon)->coordinates.lng };
	}

	pair<double, double> MapProjector::CalcMinMaxLat(const deque<const Stop*>& all_stops) const {

		const auto [min_lat, max_lat] = minmax_element(all_stops.begin(), all_stops.end(), [](const Stop* lhs, const Stop* rhs) {
			return lhs->coordinates.lat < rhs->coordinates.lat;
			});

		return { (*min_lat)->coordinates.lat, (*max_lat)->coordinates.lat };
	}

	double MapProjector::CalcScalCoordinateCoef(double min_coordinate, double max_coordinate, double axis_value) const {

		double coordinate_difference = max_coordinate - min_coordinate;
		double result_scal_coef = 0.0;

		if (coordinate_difference != 0.0) {
			result_scal_coef = (axis_value - 2 * padding_) / coordinate_difference;
		}
		return result_scal_coef;
	}

	void MapProjector::SetZoomCoef(double width_zoom_coef, double height_zoom_coef) {

		zoom_coef_ = std::min(width_zoom_coef, height_zoom_coef);

	}

	void MapProjector::SetMinLon(double longitude) {
		min_lon_ = longitude;
	}

	void MapProjector::SetMaxLat(double latitude) {
		max_lat_ = latitude;
	}

	double MapProjector::CalcXCoordinate(double longitude) const {

		return (longitude - min_lon_) * zoom_coef_ + padding_;

	}

	double MapProjector::CalcYCoordinate(double latitude) const {

		return (max_lat_ - latitude) * zoom_coef_ + padding_;
	}

	MapRenderer::MapRenderer(MapSettings customizer, MapProjector projector) :
		customizer_(std::move(customizer)), projector_(std::move(projector)) {

	}

	void MapRenderer::RenderMap(ostream& output, const map<std::string_view, Bus*>& buses_to_render) {

		projector_.CalcCoeficients(GetAllStopList(buses_to_render));

		RenderBusLines(buses_to_render);
		RenderBusNames(buses_to_render);

		map<string_view, const Stop*> all_stops_map = GetAllStopMap(buses_to_render);

		RenderStopsCircles(all_stops_map);
		RenderStopsNames(all_stops_map);

		draw_doc_.Render(output);

	}


	void MapRenderer::RenderBusLines(const map<std::string_view, Bus*>& buses_to_render) {

		double line_width = customizer_.GetLineWidth();
		const vector<svg::Color>& palette = customizer_.GetColorPalette();
		auto color = palette.begin();

		for (const auto& [name_route, route] : buses_to_render) {

			if (route->stops.empty()) {
				continue;
			}

			draw_doc_.Add(move(CreateBusLine(*route, *color, line_width)));

			color = next(color);
			if (color == palette.end()) {
				color = palette.begin();
			}

		}

	}

	void MapRenderer::RenderBusNames(const map<std::string_view, Bus*>& buses_to_render) {

		const vector<svg::Color>& palette = customizer_.GetColorPalette();
		auto color = palette.begin();

		for (const auto& [bus_name, route] : buses_to_render) {

			const auto& stops = route->stops;

			if (stops.empty()) {
				continue;
			}

			const Stop* starting_stop = stops.front();
			const Stop* final_stop = stops.back();

			draw_doc_.Add(move(CreateBusUnderlayer(starting_stop, bus_name)));
			draw_doc_.Add(move(CreateBusName(starting_stop, *color, bus_name)));

			if (starting_stop == final_stop) {
				color = next(color);
				if (color == palette.end()) {
					color = palette.begin();
				}
				continue;
			}

			draw_doc_.Add(move(CreateBusUnderlayer(final_stop, bus_name)));
			draw_doc_.Add(move(CreateBusName(final_stop, *color, bus_name)));

			color = next(color);
			if (color == palette.end()) {
				color = palette.begin();
			}

		}

	}

	void MapRenderer::RenderStopsCircles(const map<string_view, const Stop*>& all_stops) {

		double stop_radius = customizer_.GetStopRadius();

		for (const auto& [_, stop] : all_stops) {
			draw_doc_.Add(CreateStopCircle(stop, stop_radius));
		}

	}

	void MapRenderer::RenderStopsNames(const map<string_view, const Stop*>& all_stops) {

		for (const auto& [_, stop] : all_stops) {
			draw_doc_.Add(move(CreateStopUnderlayer(stop)));
			draw_doc_.Add(move(CreateStopName(stop)));
		}

	}

	svg::Polyline MapRenderer::CreateBusLine(const Bus& route, const svg::Color& line_color, double line_width) const {

		svg::Polyline route_line;
		SetBusLineProperties(route_line, line_color, line_width);

		if (route.type == BusType::Forward) {
			BuildForwardBus(route_line, route.stops);
		}
		else {
			BuildCircleBus(route_line, route.stops);
		}

		return route_line;
	}

	void MapRenderer::SetBusLineProperties(svg::Polyline& route_line, const svg::Color& color, double line_width) const {

		route_line.SetStrokeWidth(line_width);
		route_line.SetStrokeColor(color);
		route_line.SetFillColor(svg::NoneColor);
		route_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		route_line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

	}

	void MapRenderer::BuildDirectBus(svg::Polyline& route_line, const vector<Stop*>& stops) const {

		for (const Stop* stop : stops) {

			route_line.AddPoint(GetStopCoordinates(stop));

		}

	}

	void MapRenderer::BuildForwardBus(svg::Polyline& route_line, const vector<Stop*>& stops) const {

		BuildDirectBus(route_line, stops);

		size_t i = 0;
		for (auto iter = stops.rbegin(); iter != stops.rend(); ++iter) {
			if (i < 1) {
				++i;
				continue;
			}
			route_line.AddPoint(GetStopCoordinates(*iter));
		}

	}

	void MapRenderer::BuildCircleBus(svg::Polyline& route_line, const vector<Stop*>& stops) const {

		BuildDirectBus(route_line, stops);
	}

	svg::Text MapRenderer::CreateBusUnderlayer(const Stop* stop, string_view bus_name) const {

		svg::Text route_underlayer;

		SetGeneralBusNameSettings(route_underlayer, stop, bus_name);
		SetAdditionalUnderlayerSettings(route_underlayer);

		return route_underlayer;

	}

	svg::Text MapRenderer::CreateBusName(const Stop* stop, const svg::Color& color, string_view bus_name) const {

		svg::Text route_text;

		SetGeneralBusNameSettings(route_text, stop, bus_name);
		route_text.SetFillColor(color);

		return route_text;

	}

	void MapRenderer::SetGeneralBusNameSettings(svg::Text& text_element, const Stop* stop, string_view bus_name) const {

		text_element.SetPosition(GetStopCoordinates(stop));
		text_element.SetOffset(customizer_.GetBusLabelOffset());
		text_element.SetFontSize(customizer_.GetBusLabelFontSize());
		text_element.SetFontFamily("Verdana"s);
		text_element.SetFontWeight("bold"s);
		text_element.SetData(string(bus_name.begin(), bus_name.end()));

	}

	void MapRenderer::SetAdditionalUnderlayerSettings(svg::Text& text_element) const {

		text_element.SetStrokeWidth(customizer_.GetUnderlayerWidth());
		text_element.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		text_element.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		text_element.SetFillColor(customizer_.GetUnderlayerColor());
		text_element.SetStrokeColor(customizer_.GetUnderlayerColor());

	}

	svg::Circle MapRenderer::CreateStopCircle(const Stop* stop, double stop_radius) const {

		svg::Circle stop_circle;

		stop_circle.SetCenter(GetStopCoordinates(stop));
		stop_circle.SetRadius(stop_radius);
		stop_circle.SetFillColor("white");
		return stop_circle;
	}

	svg::Text MapRenderer::CreateStopUnderlayer(const Stop* stop) const {

		svg::Text stop_underlayer;
		SetGeneralStopNameSettings(stop_underlayer, stop);
		SetAdditionalUnderlayerSettings(stop_underlayer);

		return stop_underlayer;
	}

	svg::Text MapRenderer::CreateStopName(const Stop* stop) const {

		svg::Text stop_name;
		SetGeneralStopNameSettings(stop_name, stop);
		stop_name.SetFillColor("black"s);

		return stop_name;
	}

	void MapRenderer::SetGeneralStopNameSettings(svg::Text& text_element, const Stop* stop) const {

		text_element.SetPosition(GetStopCoordinates(stop));
		text_element.SetOffset(customizer_.GetStopLabelOffset());
		text_element.SetFontSize(customizer_.GetStopLabelFontSize());
		text_element.SetFontFamily("Verdana"s);
		text_element.SetData(stop->stop_name);

	}

	svg::Point MapRenderer::GetStopCoordinates(const Stop* stop) const {

		double x_coordinate = projector_.CalcXCoordinate(stop->coordinates.lng);
		double y_coordinate = projector_.CalcYCoordinate(stop->coordinates.lat);

		return svg::Point{ x_coordinate, y_coordinate };

	}


	map<string_view, const Stop*> MapRenderer::GetAllStopMap(const map<string_view, Bus*>& buses_to_render) const {

		map<string_view, const Stop*> all_stops;

		for (const auto& [bus_name, route] : buses_to_render) {

			for (const Stop* stop : route->stops) {

				if (all_stops.count(stop->stop_name) == 0) {
					all_stops.insert({ stop->stop_name, stop });
				}
			}

		}

		return all_stops;
	}

	deque<const Stop*> MapRenderer::GetAllStopList(const map<string_view, Bus*>& buses_to_render) const {

		deque<const Stop*> all_stops;

		for (const auto& [bus_name, route] : buses_to_render) {

			for (const Stop* stop : route->stops) {
				all_stops.push_back(stop);
			}

		}

		return all_stops;
	}
}