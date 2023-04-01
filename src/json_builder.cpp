#include "json_builder.h"

namespace json {

    using namespace std::literals;
    Builder& Builder::Key(std::string key) {

        if (nodes_stack_.empty()) {
            throw std::logic_error("There is no a dict for a new key"s);
        }

        const auto& last_element = nodes_stack_.back();

        if (!last_element->IsDict()) {
            throw std::logic_error("Can't insert a key into not a dict"s);
        }

        unfinished_keys_stack_.push_back(std::move(key));

        if (unfinished_keys_stack_.size() > unfinished_dicts_) {
            unfinished_keys_stack_.pop_back();
            throw std::logic_error("Can't add a new key until prev key is not completed"s);
        }

        return *this;
    }

    Builder& Builder::Value(Node::Value value) {

        if (nodes_stack_.empty()) {

            nodes_stack_.emplace_back(new Node{ std::move(ReadValue(value)) });
            return *this;

        }

        auto& last_element = nodes_stack_.back();

        if (last_element->IsArray()) {

            Array& node_as_array = last_element->AsArray();
            node_as_array.emplace_back(std::move(ReadValue(value)));

            return *this;

        }

        if (last_element->IsDict()) {

            if (unfinished_keys_stack_.empty()) {
                throw std::logic_error("Can't insert a value to a dict without key"s);
            }

            if (unfinished_keys_stack_.size() != unfinished_dicts_) {
                throw std::logic_error("Can't add a new value without key"s);
            }

            std::string& last_unfinishied_key = unfinished_keys_stack_.back();
            last_element->AsDict().insert({ std::move(last_unfinishied_key), std::move(ReadValue(value)) });
            unfinished_keys_stack_.pop_back();

            return *this;

        }

        throw std::logic_error("Can't insert a value to not an array and not a dict");
    }

    Builder::DictItemContext Builder::StartDict() {

        if (nodes_stack_.empty()) {

            Dict new_dict;
            nodes_stack_.emplace_back(new Node{ std::move(new_dict) });
            unfinished_dicts_++;
            return DictItemContext(*this);

        }

        auto& last_element = nodes_stack_.back();

        if (last_element->IsArray()) {

            Dict new_dict;
            nodes_stack_.emplace_back(new Node{ std::move(new_dict) });
            unfinished_dicts_++;

        }
        else if (last_element->IsDict()) {

            if (unfinished_keys_stack_.empty() || unfinished_keys_stack_.size() != unfinished_dicts_) {
                throw std::logic_error("Can't start a new dict without a key"s);
            }

            Dict new_dict;
            nodes_stack_.emplace_back(new Node{ std::move(new_dict) });
            unfinished_dicts_++;

        }
        else {
            throw std::logic_error("Can't start a new dict after not an array or not a dict"s);
        }

        return DictItemContext(*this);
    }

    Builder& Builder::EndDict() {

        auto& last_element = nodes_stack_.back();
        size_t nodes_stack_size = nodes_stack_.size();

        if (nodes_stack_size == 0 || !last_element->IsDict()) {
            throw std::logic_error("Can't close not a dict"s);
        }

        if (nodes_stack_size == 1) {
            unfinished_dicts_--;
            return *this;
        }

        Node dict{ std::move(*last_element) };

        if (last_element) {
            delete last_element;
        }

        nodes_stack_.pop_back();

        auto& new_last_element = nodes_stack_.back();

        if (new_last_element->IsArray()) {

            new_last_element->AsArray().push_back(std::move(dict));
            unfinished_dicts_--;
            return *this;

        }

        if (new_last_element->IsDict()) {

            if (unfinished_keys_stack_.empty()) {
                throw std::logic_error("There is no a key for this dict"s);
            }

            std::string& key_for_insert = unfinished_keys_stack_.back();
            new_last_element->AsDict().insert({ std::move(key_for_insert), std::move(dict) });
            unfinished_keys_stack_.pop_back();
        }

        unfinished_dicts_--;
        return *this;
    }

    Builder::ArrayItemContext Builder::StartArray() {

        if (nodes_stack_.empty()) {

            Array new_array;
            nodes_stack_.emplace_back(new Node{ std::move(new_array) });

            return *this;

        }

        auto& last_element = nodes_stack_.back();

        if (last_element->IsArray()) {

            Array new_array;
            nodes_stack_.emplace_back(new Node{ std::move(new_array) });

        }
        else if (last_element->IsDict()) {

            if (unfinished_keys_stack_.size() != unfinished_dicts_) {
                throw std::logic_error("Can't add a new array without key"s);
            }

            Array new_array;
            nodes_stack_.emplace_back(new Node{ std::move(new_array) });

        }
        else {
            throw std::logic_error("Can't start a new array after not an array or not a dict"s);
        }

        return ArrayItemContext{ *this };
    }

    Builder& Builder::EndArray() {

        auto& last_element = nodes_stack_.back();
        size_t nodes_stack_size = nodes_stack_.size();

        if (nodes_stack_size == 0 || !last_element->IsArray()) {
            throw std::logic_error("Can't end not an array"s);
        }

        if (nodes_stack_size == 1) {
            return *this;
        }

        Node ready_array = std::move(*last_element);

        if (last_element) {
            delete last_element;
        }

        nodes_stack_.pop_back();

        auto& new_last_element = nodes_stack_.back();

        if (new_last_element->IsArray()) {

            new_last_element->AsArray().push_back(std::move(ready_array));
            return *this;

        }

        if (new_last_element->IsDict()) {

            std::string& key_for_ready_array = unfinished_keys_stack_.back();
            new_last_element->AsDict().insert({ std::move(key_for_ready_array), std::move(ready_array) });
            unfinished_keys_stack_.pop_back();
        }

        return *this;
    }

    Node& Builder::Build() {

        if (root_.IsNull()) {

            if (nodes_stack_.empty()) {
                throw std::logic_error("Can't build a node without any value"s);
            }

            auto& ready_node = nodes_stack_.back();
            root_ = std::move(*ready_node);
            if (ready_node) {
                delete ready_node;
            }

            nodes_stack_.clear();

        }

        return root_;
    }

    Node ReadValue(Node::Value& value) {

        if (std::holds_alternative<bool>(value)) {
            return Node{ std::get<bool>(value) };
        }
        else if (std::holds_alternative<int>(value)) {
            return Node{ std::get<int>(value) };
        }
        else if (std::holds_alternative<double>(value)) {
            return Node{ std::get<double>(value) };
        }
        else if (std::holds_alternative<std::string>(value)) {
            return Node{ std::get<std::string>(value) };
        }
        else if (std::holds_alternative<Array>(value)) {
            return Node{ std::get<Array>(value) };
        }
        else if (std::holds_alternative<Dict>(value)) {
            return Node{ std::get<Dict>(value) };
        }
        else {
            return Node{ std::get<std::nullptr_t>(value) };
        }

    }

}