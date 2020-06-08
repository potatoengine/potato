// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "annotation.h"
#include "serializer.h"

#include "potato/runtime/json.h"
#include "potato/spud/string.h"
#include "potato/spud/string_format.h"
#include "potato/spud/string_writer.h"
#include "potato/spud/vector.h"

#include <nlohmann/json.hpp>

namespace up::reflex {
    struct JsonName {
        constexpr JsonName(zstring_view n) noexcept : name(n) {}

        zstring_view name;
    };

    /// Writes JSON from objects
    class JsonStreamSerializer : public SerializerBase<JsonStreamSerializer> {
    public:
        JsonStreamSerializer(nlohmann::json& root) noexcept : _root(root), _current({&_root}) {}

        friend class SerializerBase<JsonStreamSerializer>;

    protected:
        template <typename T, typename Annotations> Action enterField(tag<T>, zstring_view name, Annotations&& annotations) noexcept {
            auto extract = [this](auto const& name) { _nextField = name.name; };
            if (!ApplyAnnotation<JsonName>(annotations, extract)) {
                _nextField = name;
            }
            return Action::Enter;
        }

        template <typename T> Action enterItem(tag<T>, size_t) noexcept {
            _nextAppend = true;
            return Action::Enter;
        }

        template <typename T> Action beginObject(tag<T>) {
            _current.push_back(_assign(nlohmann::json::object()));
            return Action::Enter;
        }

        template <typename T> Action beginArray(tag<T>, size_t size) noexcept {
            _current.push_back(_assign(nlohmann::json::array()));
            return Action::Enter;
        }

        void end() noexcept { _current.pop_back(); }

        template <typename T> void handle(T&& value) { _assign(_encode(std::forward<T>(value))); }

    private:
        template <typename T> auto _encode(T&& value) -> nlohmann::json {
            using BaseT = remove_cvref_t<T>;
            if constexpr (std::is_same_v<up::string_view, BaseT>) {
                return std::string(value.begin(), value.end());
            }
            if constexpr (is_string_v<BaseT>) {
                return value.c_str();
            }
            if constexpr (is_numeric_v<BaseT>) {
                return value;
            }
            return nullptr;
        }

        auto _assign(nlohmann::json node) -> nlohmann::json* {
            auto* parent = _current.back();

            if (_nextField) {
                auto& child = (*parent)[_nextField.c_str()] = std::move(node);
                _nextField = {};
                return &child;
            }
            if (_nextAppend) {
                parent->push_back(std::move(node));
                auto& child = parent->back();
                _nextAppend = false;
                return &child;
            }

            *parent = std::move(node);
            return parent;
        }

        nlohmann::json& _root;
        vector<nlohmann::json*> _current;
        zstring_view _nextField;
        bool _nextAppend = false;
    }; // namespace up::reflex

    /// Populates objects from JSON
    class JsonStreamDeserializer : public SerializerBase<JsonStreamDeserializer> {
    public:
        explicit JsonStreamDeserializer(nlohmann::json& root) noexcept : _current({&root}) {}

        friend class SerializerBase<JsonStreamDeserializer>;

    protected:
        template <typename T, typename Annotations> Action enterField(tag<T>, zstring_view name, Annotations&& annotations) noexcept {
            zstring_view fieldName = name;
            auto extract = [&fieldName](auto const& name) { fieldName = name.name; };
            if (!ApplyAnnotation<JsonName>(annotations, extract)) {
                fieldName = name;
            }

            auto* node = _current.back();
            if (node->is_object() && node->contains(fieldName.c_str())) {
                _nextField = fieldName;
                return Action::Enter;
            }
            return Action::Skip;
        }

        template <typename T> Action enterItem(tag<T>, size_t index) noexcept {
            auto* node = _current.back();
            if (node->is_array() && index < node->size()) {
                _nextIndex = index;
                return Action::Enter;
            }
            return Action::Skip;
        }

        template <typename T> Action beginObject(tag<T>) {
            auto* node = _enter();
            if (node != nullptr && node->is_object()) {
                _current.push_back(node);
                return Action::Enter;
            }
            return Action::Skip;
        }

        template <typename T> Action beginArray(tag<T>, size_t& size) {
            auto* node = _enter();
            if (node != nullptr && node->is_array()) {
                _current.push_back(node);
                size = node->size();
                return Action::Enter;
            }
            return Action::Skip;
        }

        void end() noexcept { _current.pop_back(); }

        template <typename T> void handle(T&& value) {
            using BaseT = remove_cvref_t<T>;
            auto* node = _enter();
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
        auto _enter() -> nlohmann::json* {
            auto* parent = _current.back();
            if (_nextField && parent->is_object() && parent->contains(_nextField.c_str())) {
                auto& current = (*parent)[_nextField.c_str()];
                _nextField = {};
                return &current;
            }
            if (parent->is_array() && _nextIndex < parent->size()) {
                auto& current = (*parent)[_nextIndex];
                _nextIndex = ~size_t{};
                return &current;
            }
            return nullptr;
        }

        vector<nlohmann::json*> _current;
        zstring_view _nextField;
        size_t _nextIndex = ~size_t{};
    };
} // namespace up::reflex
