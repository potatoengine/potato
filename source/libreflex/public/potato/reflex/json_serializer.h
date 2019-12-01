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
        JsonStreamSerializer(nlohmann::json& root) noexcept : _root(root), _current({&_root}) {}

        friend class SerializerBase<JsonStreamSerializer>;

    private:
        Action enterField(zstring_view name, TypeInfo info) noexcept {
            _fieldName = name;
            return Action::Enter;
        }

        void leaveField(zstring_view name, TypeInfo info) noexcept {
            _fieldName = {};
        }

        Action enterObject(TypeInfo info) {
            if (_fieldName && !_current.empty()) {
                auto& obj = (*_current.back())[_fieldName.c_str()] = nlohmann::json::object();
                _current.push_back(&obj);
            }
            return Action::Enter;
        }

        void leaveObject(TypeInfo info) noexcept {
            _current.pop_back();
        }

        template <typename Tag, typename Getter, typename Setter>
        void dispatch(Tag, Getter getter, Setter&&) {
            auto&& tmp = getter();
            serialize(tmp, *this);
        }

        template <typename T>
        void handle(T&& value) {
            using BaseT = remove_cvref_t<T>;
            if (_fieldName && !_current.empty()) {
                if constexpr (std::is_same_v<up::string_view, BaseT>) {
                    (*_current.back())[_fieldName.c_str()] = std::string(value.begin(), value.end());
                }
                else if constexpr (is_string_v<BaseT>) {
                    (*_current.back())[_fieldName.c_str()] = value.c_str();
                }
                else if constexpr (is_numeric_v<BaseT>) {
                    (*_current.back())[_fieldName.c_str()] = value;
                }
                else {
                    (*_current.back())[_fieldName.c_str()] = nullptr;
                }
            }
        }

    private:
        nlohmann::json& _root;
        vector<nlohmann::json*> _current;
        zstring_view _fieldName;
    };

    /// Populates objects from JSON
    class JsonStreamDeserializer : public SerializerBase<JsonStreamDeserializer> {
    public:
        JsonStreamDeserializer(nlohmann::json& root) noexcept : _root(root), _current({&_root}) {}

        friend class SerializerBase<JsonStreamDeserializer>;

    private:
        Action enterField(zstring_view name, TypeInfo info) noexcept {
            if (current().contains(name.c_str())) {
                _fieldName = name;
                return Action::Enter;
            }
            return Action::Skip;
        }

        constexpr void leaveField(zstring_view name, TypeInfo info) noexcept {
            _fieldName = {};
        }

        Action enterObject(TypeInfo info) {
            if (_fieldName && !_current.empty() && current().is_object()) {
                auto& obj = current()[_fieldName.c_str()];
                if (obj.is_object()) {
                    _current.push_back(&obj);
                    return Action::Enter;
                }
            }
            return Action::Skip;
        }

        void leaveObject(TypeInfo info) noexcept {
            _current.pop_back();
        }

        template <typename T, typename Getter, typename Setter>
        void dispatch(tag<T>, Getter&&, Setter setter) {
            auto tmp = current()[_fieldName.c_str()].get<T>();
            setter(std::move(tmp));
        }

        template <typename T>
        void handle(T&& value) {
            using BaseT = remove_cvref_t<T>;
            if constexpr (is_numeric_v<BaseT>) {
                if (_fieldName && !_current.empty() && current().is_object()) {
                    auto& field = current()[_fieldName.c_str()];
                    if (field.is_primitive()) {
                        value = current()[_fieldName.c_str()].get<BaseT>();
                    }
                }
            }
        }

        void handle(up::string& value) {
            if (_fieldName && !_current.empty() && current().is_object()) {
                auto& field = current()[_fieldName.c_str()];
                if (field.is_string()) {
                    value = current()[_fieldName.c_str()].get<up::string>();
                }
            }
        }

        nlohmann::json& current() noexcept {
            return *_current.back();
        }

        nlohmann::json& _root;
        vector<nlohmann::json*> _current;
        zstring_view _fieldName;
    };
} // namespace up::reflex
