// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#pragma once

#include "serializer.h"
#include <potato/spud/string_writer.h>
#include <potato/spud/string_format.h>
#include <potato/spud/string.h>
#include <potato/spud/vector.h>
#include <nlohmann/json.hpp>

// temporary - FIXME remove
namespace up {
    static inline void from_json(const nlohmann::json& json, up::string& str) noexcept {
        str.reset();
        if (json.is_string()) {
            const std::string& stdString = json;
            str = up::string(stdString.data(), stdString.size());
        }
    }
} // namespace up

namespace up::reflex {
    /// Writes JSON from objects
    class JsonStreamSerializer : public SerializerBase<JsonStreamSerializer> {
    public:
        JsonStreamSerializer(nlohmann::json& root) noexcept : _root(root), _current() {}

        friend class SerializerBase<JsonStreamSerializer>;

    protected:
        template <typename T>
        Action enterField(tag<T>, zstring_view name) noexcept {
            _fieldName = name;
            return Action::Enter;
        }

        template <typename T>
        Action enterItem(tag<T>, size_t) noexcept {
            _append = true;
            return Action::Enter;
        }

        template <typename T>
        Action beginObject(tag<T>) {
            _current.push_back(_assign(nlohmann::json::object()));
            return Action::Enter;
        }

        template <typename T>
        Action beginArray(tag<T>, size_t size) noexcept {
            _current.push_back(_assign(nlohmann::json::array()));
            return Action::Enter;
        }

        void end() noexcept {
            _current.pop_back();
        }

        template <typename T>
        void handle(T&& value) {
            _assign(_encode(std::forward<T>(value)));
        }

    private:
        template <typename T>
        auto _encode(T&& value) -> nlohmann::json {
            using BaseT = remove_cvref_t<T>;
            if constexpr (std::is_same_v<up::string_view, BaseT>) {
                return std::string(value.begin(), value.end());
            }
            else if constexpr (is_string_v<BaseT>) {
                return value.c_str();
            }
            else if constexpr (is_numeric_v<BaseT>) {
                return value;
            }
            else {
                return nullptr;
            }
        }

        auto _assign(nlohmann::json node) -> nlohmann::json* {
            auto& parent = !_current.empty() ? *_current.back() : _root;

            if (_fieldName) {
                auto& child = parent[_fieldName.c_str()] = std::move(node);
                _fieldName = {};
                return &child;
            }
            else if (_append) {
                parent.push_back(std::move(node));
                auto& child = parent.back();
                _append = false;
                return &child;
            }
            else {
                _root = std::move(node);
                return &_root;
            }
        }

    private:
        nlohmann::json& _root;
        vector<nlohmann::json*> _current;
        zstring_view _fieldName;
        bool _append = false;
    }; // namespace up::reflex

    /// Populates objects from JSON
    class JsonStreamDeserializer : public SerializerBase<JsonStreamDeserializer> {
    public:
        JsonStreamDeserializer(nlohmann::json& root) noexcept : _root(root), _current() {}

        friend class SerializerBase<JsonStreamDeserializer>;

    protected:
        template <typename T>
        Action enterField(tag<T>, zstring_view name) noexcept {
            if (_current.back()->is_object() && _current.back()->contains(name.c_str())) {
                _fieldName = name;
                return Action::Enter;
            }
            return Action::Skip;
        }

        template <typename T>
        Action enterItem(tag<T>, size_t index) noexcept {
            if (_current.back()->is_array() && index < _current.back()->size()) {
                _index = index;
                return Action::Enter;
            }
            return Action::Skip;
        }

        template <typename T>
        Action beginObject(tag<T>) {
            auto* node = current();
            if (node != nullptr && node->is_object()) {
                _current.push_back(node);
                return Action::Enter;
            }
            return Action::Skip;
        }

        template <typename T>
        Action beginArray(tag<T>, size_t& size) noexcept {
            auto* node = current();
            if (node != nullptr && node->is_array()) {
                _current.push_back(node);
                size = node->size();
                return Action::Enter;
            }
            return Action::Skip;
        }

        void end() noexcept {
            _current.pop_back();
        }

        template <typename T>
        void handle(T&& value) {
            using BaseT = remove_cvref_t<T>;
            auto* node = current();
            if (node != nullptr) {
                if constexpr (std::is_same_v<BaseT, up::string>) {
                    if (node->is_string()) {
                        value = node->get<up::string>();
                    }
                }
                else if constexpr (is_numeric_v<BaseT>) {
                    if (node->is_number()) {
                        value = node->get<BaseT>();
                    }
                }
            }
        }

    private:
        auto current() noexcept -> nlohmann::json* {
            auto& parent = !_current.empty() ? *_current.back() : _root;

            if (_fieldName && parent.is_object() && parent.contains(_fieldName.c_str())) {
                auto& current = parent[_fieldName.c_str()];
                _fieldName = {};
                return &current;
            }
            else if (parent.is_array() && _index < parent.size()) {
                auto& current = parent[_index];
                _index = ~size_t{};
                return &current;
            }
            else {
                return nullptr;
            }
        }

    private:
        nlohmann::json& _root;
        vector<nlohmann::json*> _current;
        zstring_view _fieldName;
        size_t _index = ~size_t{};
    };
} // namespace up::reflex
