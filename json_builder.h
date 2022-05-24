#pragma once

#include "json.h"

namespace json {


    Node ReadValue(Node::Value& value);


    class Builder {
    public:

        class BaseItemContext;
        class KeyItemContext;
        class DictItemContext;
        class ArrayItemContext;

        class BaseItemContext {
        public:
            BaseItemContext(Builder& builder) : json_builder_(builder) {

            }

            Builder& Value(Node::Value value) {
                return json_builder_.Value(std::move(value));
            }

            KeyItemContext Key(std::string key, DictItemContext& parent) {
                json_builder_.Key(std::move(key));
                return KeyItemContext(json_builder_, parent);
            }

            DictItemContext StartDict() {
                return json_builder_.StartDict();
            }

            ArrayItemContext StartArray() {
                return json_builder_.StartArray();
            }

            Builder& EndDict() {
                return json_builder_.EndDict();
            }

            Builder& EndArray() {
                return json_builder_.EndArray();
            }

            Builder& GetBuilder() {
                return json_builder_;
            }

        private:
            Builder& json_builder_;

        };

        class DictItemContext : public BaseItemContext {
        public:
            using BaseItemContext::BaseItemContext;

            KeyItemContext Key(std::string key) {
                return BaseItemContext::Key(std::move(key), *this);
            }

            Builder& Value(Node::Value) = delete;
            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
            Builder& EndArray() = delete;

        private:

        };


        class KeyItemContext : public BaseItemContext {
        public:
            KeyItemContext(Builder& builder, DictItemContext& parent) : BaseItemContext(builder), parent_(parent) {

            }

            KeyItemContext Key(std::string) = delete;
            DictItemContext& Value(Node::Value value) {
                BaseItemContext::Value(value);
                return parent_;
            }

            Builder& EndDict() = delete;
            Builder& EndArray() = delete;

        private:
            DictItemContext& parent_;
        };


        class ArrayItemContext : public BaseItemContext {
        public:
            using BaseItemContext::BaseItemContext;

            KeyItemContext Key(std::string) = delete;
            Builder& EndDict() = delete;

            ArrayItemContext& Value(Node::Value value) {
                BaseItemContext::Value(std::move(value));
                return *this;
            }

        private:

        };

        Builder() = default;
        Builder& Key(std::string);
        Builder& Value(Node::Value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Node& Build();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        std::vector<std::string> unfinished_keys_stack_;
        size_t unfinished_dicts_ = 0;

    };

}