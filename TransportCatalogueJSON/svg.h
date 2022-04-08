#pragma once

#define _USE_MATH_DEFINES
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <cmath>
#include <optional>
#include <string_view>
#include <variant>

namespace svg {

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t red, uint8_t green, uint8_t blue);

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;

    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity);

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };


    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    std::ostream& operator<<(std::ostream& os, const Color& color);

    struct ColorVisitor {

        std::ostream& os;
        void operator()(const Rgb& rgb) {
            using namespace std::literals;
            os << "rgb("sv << static_cast<size_t>(rgb.red) << ","sv << static_cast<size_t>(rgb.green) << ","sv << static_cast<size_t>(rgb.blue) << ")"sv;
        }
        void operator()(const Rgba& rgba) {
            using namespace std::literals;
            os << "rgba("sv << static_cast<size_t>(rgba.red) << ","sv << static_cast<size_t>(rgba.green) << ","sv << static_cast<size_t>(rgba.blue) << ","sv << rgba.opacity << ")"sv;
        }
        void operator()(std::monostate) {
            using namespace std::literals;
            os << ""sv;
        }
        void operator()(const std::string& color) {
            using namespace std::literals;
            os << color;
        }
    };

    inline const Color NoneColor{ "none" };


    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& stroke_line_cap);

    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& stroke_line_join);

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;
        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_line_cap_) {
                out << " stroke-linecap=\""sv << stroke_line_cap_.value() << "\""sv;
            }
            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv << stroke_line_join_.value() << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };


    class ObjectContainer {
    public:
        template<typename Obj>
        void Add(Obj object);

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        virtual ~ObjectContainer() = default;
    };

    // ---------- ObjectContainer ----------

    template<typename Obj>
    void ObjectContainer::Add(Obj object) {
        AddPtr(std::make_unique<Obj>(std::move(object)));
    }

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& obj_container) const = 0;
        virtual ~Drawable() = default;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_ = { 0.0, 0.0 };
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> polyline_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

        // Прочие данные и методы, необходимые для реализации элемента <text>
    private:
        void RenderObject(const RenderContext& context) const override;

        Point control_point_ = { 0.0, 0.0 };
        Point control_point_offset_ = { 0, 0 };
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;

    };

    class Document : public ObjectContainer {
    public:
        /*
         Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
         Пример использования:
         Document doc;
         doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
        */
        // void Add(???);
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

        // Прочие методы и данные, необходимые для реализации класса Document
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg
