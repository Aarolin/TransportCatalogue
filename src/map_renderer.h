#pragma once

#include "json.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <deque>
#include <execution>
#include <map>
#include <vector>

namespace render {


	class MapSettings {
	public:
		explicit MapSettings(const json::Dict&);
		MapSettings() = default;

		void SetWidth(double);
		double GetWidth() const;

		void SetHeight(double);
		double GetHeight() const;

		void SetPadding(double);
		double GetPadding() const;

		void SetLineWidth(double);
		double GetLineWidth() const;

		void SetStopRadius(double);
		double GetStopRadius() const;

		void SetBusLabelFontSize(int);
		int GetBusLabelFontSize() const;

		void SetBusLabelOffset(const json::Array&);
		void SetBusLabelOffset(svg::Point p);

		svg::Point GetBusLabelOffset() const;

		void SetStopLabelFontSize(int);
		int GetStopLabelFontSize() const;

		void SetStopLabelOffset(const json::Array&);
		void SetStopLabelOffset(svg::Point p);

		svg::Point GetStopLabelOffset() const;

		void SetUnderlayerColor(const json::Node&);
		void SetUnderlayerColor(svg::Color);

		const svg::Color& GetUnderlayerColor() const;

		void SetUnderlayerWidth(double);
		double GetUnderlayerWidth() const;

		void SetColorPalette(const json::Array&);
		void SetColorPalette(std::vector<svg::Color> color_palette);

		const std::vector<svg::Color>& GetColorPalette() const;

	private:

		svg::Color GetColorByNode(const json::Node& node_color) const;

		double width_ = 0;
		double height_ = 0;
		double padding_ = 0;
		double line_width_ = 0;
		double stop_radius_ = 0;
		int bus_label_font_size_ = 0;
		svg::Point bus_label_offset_;
		int stop_label_font_size_ = 0;
		svg::Point stop_label_offset_;
		svg::Color underlayer_color_;
		double underlayer_width_ = 0;
		std::vector<svg::Color> color_palette_;
	};

	class MapProjector {

	public:

		MapProjector(double, double, double);
		MapProjector() = default;

		void CalcCoeficients(const std::deque<const Stop*>& all_stops);

		double GetWidth() const;
		double GetHeight() const;

		double CalcXCoordinate(double longitude) const;
		double CalcYCoordinate(double latitude) const;

	private:

		std::pair<double, double> CalcMinMaxLong(const std::deque<const Stop*>& all_stops) const;
		std::pair<double, double> CalcMinMaxLat(const std::deque<const Stop*>& all_stops) const;

		double CalcScalCoordinateCoef(double min_coordinate, double max_coordinate, double axis_value) const;

		void SetZoomCoef(double width_zoom_coef, double height_zoom_coef);
		void SetMinLon(double min_lon);
		void SetMaxLat(double max_lat);

		double width_ = 0.0;
		double height_ = 0.0;
		double padding_ = 0.0;

		double min_lon_ = 0.0;
		double max_lat_ = 0.0;

		double zoom_coef_ = 0.0;

	};

	class MapRenderer {
	public:
		MapRenderer() = default;
		MapRenderer(MapSettings customizer, MapProjector projector);

		void RenderMap(std::ostream& output, const std::map<std::string_view, Bus*>& routes_to_render);

	private:

		void RenderBusLines(const std::map<std::string_view, Bus*>& buses_to_render);
		void RenderBusNames(const std::map<std::string_view, Bus*>& buses_to_render);
		void RenderStopsCircles(const std::map<std::string_view, const Stop*>& all_stops);
		void RenderStopsNames(const std::map<std::string_view, const Stop*>& all_stops);

		svg::Polyline CreateBusLine(const Bus& bus, const svg::Color& line_color, double line_width) const;

		void SetBusLineProperties(svg::Polyline& bus_line, const svg::Color& color, double line_width) const;

		void BuildDirectBus(svg::Polyline& bus_line, const std::vector<Stop*>& stops) const;

		void BuildForwardBus(svg::Polyline& bus_line, const std::vector<Stop*>& stops) const;
		void BuildCircleBus(svg::Polyline& bus_line, const std::vector<Stop*>& stops) const;

		svg::Text CreateBusUnderlayer(const Stop* stop, std::string_view bus_name) const;
		svg::Text CreateBusName(const Stop* stop, const svg::Color& color, std::string_view bus_name) const;

		void SetGeneralBusNameSettings(svg::Text& text_element, const Stop* stop, std::string_view bus_name) const;
		void SetAdditionalUnderlayerSettings(svg::Text& text_element) const;

		svg::Circle CreateStopCircle(const Stop* stop, double stop_radius) const;

		svg::Text CreateStopUnderlayer(const Stop* stop) const;
		svg::Text CreateStopName(const Stop* stop) const;

		void SetGeneralStopNameSettings(svg::Text& text_element, const Stop* stop) const;

		svg::Point GetStopCoordinates(const Stop* stop) const;

		std::map<std::string_view, const Stop*> GetAllStopMap(const std::map<std::string_view, Bus*>& buses_to_render) const;
		std::deque<const Stop*> GetAllStopList(const std::map<std::string_view, Bus*>& buses_to_render) const;

		MapSettings customizer_;
		MapProjector projector_;
		svg::Document draw_doc_;

	};

}