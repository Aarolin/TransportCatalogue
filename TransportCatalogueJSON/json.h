#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        Node() = default;
        Node(Array array);
        Node(Dict map);
        Node(int value);
        Node(double value);
        Node(std::nullptr_t ptr);
        Node(bool value);
        Node(std::string value);

        bool IsNull() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsMap() const;

        bool operator==(const Node& rhs) const;
        bool operator!=(const Node& rhs) const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const std::string& AsString() const;

    private:
        std::variant<std::nullptr_t, bool, int, double, Array, Dict, std::string> element_;
    };

    class Document {
    public:
        explicit Document(Node root);

        bool operator==(const Document& rhs) const;
        bool operator!=(const Document& rhs) const;

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);


}  // namespace json