#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            if (input.peek() == ']') {
                input.get();
                return result;
            }

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                try {
                    result.push_back(LoadNode(input));
                }
                catch (ParsingError& er) {
                    throw ParsingError(er.what());
                }
            }

            if (result.empty()) {
                throw ParsingError("Failed to parse an array");
            }

            return Node(move(result));
        }

        using Number = std::variant<int, double>;

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            if (input.peek() == '0') {
                read_char();
            }
            else {
                read_digits();
            }

            bool is_int = true;
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }


        Node LoadString(istream& input) {
            string line;
            char c;
            while (input.get(c)) {
                if (c == '\\') {
                    char next_symbol;
                    input.get(next_symbol);
                    if (next_symbol == '\\') {
                        line += '\\';
                        continue;
                    }
                    else if (next_symbol == 'r') {
                        line += '\r';
                        continue;
                    }
                    else if (next_symbol == 'n') {
                        line += '\n';
                        continue;
                    }
                    else if (next_symbol == 't') {
                        line += '\t';
                        continue;
                    }
                    else if (next_symbol == '\"') {
                        line += '\"';
                        continue;
                    }
                }
                else {
                    if (c == '\"') {
                        break;
                    }
                }
                line += c;
            }
            if (c != '\"') {
                throw ParsingError("Failed to parse string node: "s + line);
            }
            return Node(move(line));
        }

        Node LoadDict(istream& input) {

            Dict result;

            if (input.peek() == '}') {

                input.get();
                return result;

            }

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }
                try {
                    string key = LoadString(input).AsString();
                    input >> c;
                    result.insert({ move(key), LoadNode(input) });
                }
                catch (ParsingError& er) {
                    throw ParsingError(er.what());
                }
            }

            if (result.empty()) {
                throw ParsingError("Failed to parse a map");
            }

            return Node(move(result));
        }

        Node LoadBool(istream& input) {
            char c;
            string boolean;
            if (input.peek() == 't') {
                for (int i = 0; i < 4; ++i) {
                    input.get(c);
                    boolean += c;
                }
            }
            else {
                for (int i = 0; i < 5; ++i) {
                    input.get(c);
                    boolean += c;
                }
            }

            if (boolean != "true" && boolean != "false") {
                throw ParsingError("Failed parse to bool");
            }

            if (boolean == "true") {
                return Node{ true };
            }

            return Node{ false };
        }

        Node LoadNull(istream& input) {

            string nullstr;
            char c;

            for (int i = 0; i < 4 && input.get(c); ++i) {
                nullstr += c;
            }

            if (nullstr != "null") {
                throw ParsingError("Failed to parse null");
            }

            return Node{ nullptr };
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 't') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else {
                input.putback(c);
                try {
                    Number num = LoadNumber(input);
                    if (holds_alternative<int>(num)) {
                        return Node(get<int>(num));
                    }
                    return Node(get<double>(num));
                }
                catch (ParsingError& er) {
                    throw ParsingError(er.what());
                }
            }
        }

    }  // namespace

    Node::Node(nullptr_t ptr)
        : element_(ptr) {

    }
    Node::Node(Array array)
        : element_(move(array)) {
    }

    Node::Node(Dict map)
        : element_(move(map)) {
    }

    Node::Node(int value)
        : element_(value) {
    }

    Node::Node(double value)
        : element_(value) {

    }

    Node::Node(bool value)
        : element_(value) {

    }

    Node::Node(string value)
        : element_(move(value)) {
    }

    bool Node::IsNull() const {
        if (holds_alternative<std::nullptr_t>(element_)) {
            return true;
        }
        return false;
    }

    bool Node::IsBool() const {
        if (holds_alternative<bool>(element_)) {
            return true;
        }
        return false;
    }

    bool Node::IsInt() const {
        if (holds_alternative<int>(element_)) {
            return true;
        }
        return false;
    }

    bool Node::IsDouble() const {
        if (holds_alternative<double>(element_) || IsInt()) {
            return true;
        }
        return false;
    }

    bool Node::IsPureDouble() const {
        if (holds_alternative<double>(element_)) {
            return true;
        }
        return false;
    }

    bool Node::IsString() const {
        if (holds_alternative<std::string>(element_)) {
            return true;
        }
        return false;
    }

    bool Node::IsArray() const {
        if (holds_alternative<Array>(element_)) {
            return true;
        }
        return false;
    }

    bool Node::IsMap() const {
        if (holds_alternative<Dict>(element_)) {
            return true;
        }
        return false;
    }

    bool Node::operator==(const Node& rhs) const {
        return rhs.element_ == element_;
    }

    bool Node::operator!=(const Node& rhs) const {
        return !(rhs.element_ == element_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("Node is not an array"s);
        }

        return get<Array>(element_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("Node is not a map"s);
        }

        return get<Dict>(element_);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("Node is not an int"s);
        }

        return get<int>(element_);
    }

    double Node::AsDouble() const {
        if (!IsDouble() && !IsInt()) {
            throw std::logic_error("Node is not a double"s);
        }

        if (const auto* value = get_if<int>(&element_)) {
            const int num = *value;
            return static_cast<double>(num);
        }

        return get<double>(element_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("Node is not a bool"s);
        }

        return get<bool>(element_);
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("Node is not a string"s);
        }

        return get<std::string>(element_);
    }


    Document::Document(Node root)
        : root_(move(root)) {
    }

    bool Document::operator==(const Document& rhs) const {
        return rhs.root_ == root_;
    }

    bool Document::operator!=(const Document& rhs) const {
        return rhs.root_ != root_;
    }
    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        const Node& node = doc.GetRoot();
        if (node.IsInt()) {
            output << node.AsInt();
            return;
        }
        else if (node.IsDouble()) {
            output << node.AsDouble();
            return;
        }
        else if (node.IsBool()) {
            output << std::boolalpha << node.AsBool();
            return;
        }
        else if (node.IsString()) {
            string line;
            line += '\"';
            for (const char c : node.AsString()) {
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
            output << line;
            return;
        }
        else if (node.IsArray()) {
            const Array& arr = node.AsArray();
            output << "[";
            size_t arr_size = arr.size();
            size_t i = 0;

            for (const Node& node : arr) {
                Print(Document(node), output);
                if (i != arr_size - 1) {
                    output << ", ";
                }
                ++i;
            }

            output << "]";
            return;

        }
        else if (node.IsMap()) {
            const Dict& map = node.AsMap();
            output << "{" << std::endl;
            size_t map_size = map.size();
            size_t i = 0;

            for (const auto& [key, value] : map) {
                output << "    ";
                output << '\"' << key << '\"' << ": ";
                Print(Document(value), output);
                if (i != map_size - 1) {
                    output << ',' << std::endl;
                }
                ++i;
            }

            output << "}";
            return;
        }
        else {
            output << "null";
        }

    }

}  // namespace json