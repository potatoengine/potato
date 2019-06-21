// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#include "_export.h"
#include "metadata.h"
#include <potato/foundation/string_writer.h>
#include <potato/foundation/string_format.h>
#include <potato/foundation/string.h>
#include <potato/foundation/vector.h>
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

    template <typename T>
    constexpr bool is_string_v = std::is_same_v<T, up::string_view> || std::is_same_v<T, up::zstring_view> || std::is_same_v<T, up::string>;
} // namespace up

namespace up::reflex {
    template <typename DerivedType>
    class SerializerBase {
    public:
        enum class Action {
            Enter,
            Skip
        };

        template <typename ObjectType, typename ClassType, typename FieldType>
        constexpr void field(zstring_view name, ObjectType& object, FieldType ClassType::*field) {
            if (static_cast<DerivedType*>(this)->enterField(name) == Action::Enter) {
                value(object.*field);
                static_cast<DerivedType*>(this)->leaveField();
            }
        }

        template <typename ValueType>
        void value(ValueType&& value) {
            using Type = up::remove_cvref_t<ValueType>;
            if constexpr (is_numeric_v<Type> || is_string_v<Type>) {
                static_cast<DerivedType*>(this)->handle(value);
            }
            else if constexpr (std::is_class_v<Type>) {
                if (static_cast<DerivedType*>(this)->enterObject() == Action::Enter) {
                    serialize(value, *static_cast<DerivedType*>(this));
                    static_cast<DerivedType*>(this)->leaveObject();
                }
            }
            else {
                static_cast<DerivedType*>(this)->value(value);
            }
        }
    };

    class JsonStreamSerializer : public SerializerBase<JsonStreamSerializer> {
    public:
        JsonStreamSerializer(nlohmann::json& root) noexcept : _root(root), _current({&_root}) {}

        friend class SerializerBase<JsonStreamSerializer>;

    private:
        Action enterField(zstring_view name) noexcept {
            _fieldName = name;
            return Action::Enter;
        }

        void leaveField() noexcept {
            _fieldName = {};
        }

        Action enterObject() {
            if (_fieldName && !_current.empty()) {
                auto& obj = (*_current.back())[_fieldName.c_str()] = nlohmann::json::object();
                _current.push_back(&obj);
            }
            return Action::Enter;
        }

        void leaveObject() noexcept {
            _current.pop_back();
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
            }
        }

    private:
        nlohmann::json& _root;
        vector<nlohmann::json*> _current;
        zstring_view _fieldName;
    };

    class JsonStreamDeserializer : public SerializerBase<JsonStreamDeserializer> {
    public:
        JsonStreamDeserializer(nlohmann::json& root) noexcept : _root(root), _current({&_root}) {}

        friend class SerializerBase<JsonStreamDeserializer>;

    private:
        Action enterField(zstring_view name) noexcept {
            if (current().contains(name.c_str())) {
                _fieldName = name;
                return Action::Enter;
            }
            return Action::Skip;
        }

        constexpr void leaveField() noexcept {
            _fieldName = {};
        }

        Action enterObject() {
            if (_fieldName && !_current.empty() && current().is_object()) {
                auto& obj = current()[_fieldName.c_str()];
                if (obj.is_object()) {
                    _current.push_back(&obj);
                    return Action::Enter;
                }
            }
            return Action::Skip;
        }

        void leaveObject() noexcept {
            _current.pop_back();
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
