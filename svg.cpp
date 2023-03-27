#include "svg.h"

namespace svg {

    using namespace std::literals;

    bool Rgb::operator==(const Rgb& rhs) const {
        return (red == rhs.red && green == rhs.green && blue == rhs.blue);
    }

    bool Rgba::operator==(const Rgba& rhs) const {
        return (red == rhs.red && green == rhs.green && blue == rhs.blue && opacity == rhs.opacity);
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        RenderObject(context);

        context.out << std::endl;
    }

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& stroke_line_cap) {
        using namespace std::literals;
        switch (stroke_line_cap) {
        case StrokeLineCap::BUTT:
            os << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            os << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            os << "square"sv;
            break;
        default:
            os << ""sv;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& stroke_line_join) {
        using namespace std::literals;
        switch (stroke_line_join) {
        case StrokeLineJoin::ARCS:
            os << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            os << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            os << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            os << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            os << "round"sv;
            break;
        default:
            os << ""sv;
        }
        return os;
    }

    Rgb::Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {

    }
    Rgba::Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) : red(red), green(green), blue(blue), opacity(opacity) {

    }

    std::ostream& operator<<(std::ostream& os, const Color& color) {
        std::visit(ColorVisitor{ os }, color);
        return os;
    }

    bool Point::operator==(const Point& rhs) const {
        return (x == rhs.x && y == rhs.y);
    }
    // ---------- Circle ----------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }


    // ------------- Polyline -------------

    Polyline& Polyline::AddPoint(Point point) {
        polyline_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        int size = polyline_.size();
        for (int i = 0; i < size - 1; ++i) {
            out << polyline_[i].x << ","sv << polyline_[i].y << " "sv;
        }
        if (size != 0) {
            out << polyline_[size - 1].x << ","sv << polyline_[size - 1].y;
        }
        out << "\""sv;
        RenderAttrs(out);
        out << " />"sv;
    }


    // -------------- Text --------------

    Text& Text::SetPosition(Point pos) {
        control_point_ = std::move(pos);
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        control_point_offset_ = std::move(offset);
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << control_point_.x << "\""sv << " y=\""sv << control_point_.y
            << "\""sv << " dx=\""sv << control_point_offset_.x << "\""sv << " dy=\""sv
            << control_point_offset_.y << "\""sv << " font-size=\""sv << font_size_ << "\""sv;

        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        RenderAttrs(out);
        out << ">"sv;


        for (const char& ch : data_) {

            if (ch == '\"') {
                out << "&quot"sv;
            }
            else if (ch == '<') {
                out << "&lt";
            }
            else if (ch == '>') {
                out << "&gt";
            }
            else if (ch == '&') {
                out << "&amp"sv;
            }
            else if (ch == '`' || ch == '\'') {
                out << "&apos"sv;
            }
            else {
                out << ch;
            }

        }

        out << "</text>"sv;
    }

    // ------------- Document -------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {

        RenderContext context(out);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;

        for (const auto& obj : objects_) {
            obj->Render(context);
        }
        out << "</svg>"sv;
    }
}  // namespace svg