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
    using NodeVariant = std::variant<std::nullptr_t, bool, int, double, Array, Dict, std::string>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node : public NodeVariant {
    public:
       
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

    std::ostream& operator<<(std::ostream& os, const Node& node);

    struct NodeVisitor {

        std::ostream& os;

        void operator()(std::nullptr_t) {

            using namespace std::literals;
            os << "null"sv;
        }

        void operator()(const std::string& str) {

            std::string line;
            line += '\"';
            for (const char& c : str) {
                if (c == '\"') {
                    line += '\\';
                    line += '\"';
                    continue;
                }
                else if (c == '\n') {
                    line += '\\';
                    line += 'n';
                    continue;
                }
                else if (c == '\r') {
                    line += '\\';
                    line += 'r';
                    continue;
                }
                else if (c == '\\') {
                    line += '\\';
                    line += '\\';
                    continue;
                }
                line += c;
            }
            line += '\"';
            os << line;
        }

        void operator()(bool expression) {
            os << std::boolalpha << expression;
        }

        void operator()(int num) {
            os << num;
        }

        void operator()(double num) {
            os << num;
        }

        void operator()(const Dict& dict) {

            os << "{" << std::endl;
            size_t map_size = dict.size();
            size_t i = 0;

            for (const auto& [key, value] : dict) {
                os << "    ";
                os << '\"' << key << '\"' << ": ";
                Print(Document(value), os);
                if (i != map_size - 1) {
                    os << ',' << std::endl;
                }
                ++i;
            }

            os << "}";
        }

        void operator()(const Array& arr) {

            os << "[";
            size_t arr_size = arr.size();
            size_t i = 0;

            for (const Node& node : arr) {
                Print(Document(node), os);
                if (i != arr_size - 1) {
                    os << ", ";
                }
                ++i;
            }

            os << "]";
        }

    };



}  // namespace json