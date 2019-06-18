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
}

namespace up::reflex {
    template <typename DerivedType>
    class SerializerBase  {
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
            if constexpr (std::is_integral_v<Type> || std::is_floating_point_v<Type>) {
                static_cast<DerivedType*>(this)->primitive(value);
            }
            else if constexpr (std::is_same_v<Type, up::string_view> || std::is_same_v<Type, up::zstring_view> || std::is_same_v<Type, string>) {
                static_cast<DerivedType*>(this)->string(value);
            }
            else if constexpr (std::is_class_v<Type>) {
                if (static_cast<DerivedType*>(this)->enterObject() == Action::Enter) {
                    serialize(value, *this);
                    static_cast<DerivedType*>(this)->leaveObject();
                }
            }
        }
    };

    class StreamSerializer {
    public:
        enum class Action {
            Enter,
            Skip
        };

        template <typename ObjectType, typename ClassType, typename FieldType>
        constexpr void field(zstring_view name, ObjectType& object, FieldType ClassType::*field) {
            if (enterField(name) == Action::Enter) {
                recurse(object.*field);
                leaveField();
            }
        }

        void value(int value) { primitive(value); }
        void value(float value) { primitive(value); }
        void value(string_view value) { string(value); }
        void value(string& value) { string(value); }

    private:
        template <typename T>
        constexpr void recurse(T& value) {
            if constexpr (std::is_class_v<up::remove_cvref_t<T>> && !std::is_same_v<up::remove_cvref_t<T>, string_view>) {
                if (enterObject() == Action::Enter) {
                    serialize(value, *this);
                    leaveObject();
                }
            }
            else {
                serialize(value, *this);
            }
        }

        virtual Action enterField(zstring_view name) { return Action::Skip; }
        virtual void leaveField() {}

        virtual Action enterObject() { return Action::Skip; }
        virtual void leaveObject() {}

        virtual void primitive(int) {}
        virtual void primitive(float) {}
        virtual void primitive(double value) {}

        virtual void string(string_view value) {}
        virtual void string(up::string& value) {}
    }; // namespace up::reflect

    class JsonStreamSerializer : public StreamSerializer {
    public:
        JsonStreamSerializer(nlohmann::json& root) noexcept : _root(root), _current({&_root}) {}

    private:
        virtual Action enterField(zstring_view name) {
            _fieldName = name;
            return Action::Enter;
        }

        virtual void leaveField() {
            _fieldName = {};
        }

        virtual Action enterObject() {
            if (_fieldName && !_current.empty()) {
                auto& obj = (*_current.back())[_fieldName.c_str()] = nlohmann::json::object();
                _current.push_back(&obj);
            }
            return Action::Enter;
        }

        virtual void leaveObject() {
            _current.pop_back();
        }

        void primitive(int value) override {
            if (_fieldName && !_current.empty()) {
                (*_current.back())[_fieldName.c_str()] = value;
            }
        }

        void primitive(float value) override {
            if (_fieldName && !_current.empty()) {
                (*_current.back())[_fieldName.c_str()] = value;
            }
        }

        void primitive(double value) override {
            if (_fieldName && !_current.empty()) {
                (*_current.back())[_fieldName.c_str()] = value;
            }
        }

        void string(string_view value) override {
            if (_fieldName && !_current.empty()) {
                (*_current.back())[_fieldName.c_str()] = std::string(value.begin(), value.end());
            }
        }

        void string(up::string& value) override {
            if (_fieldName && !_current.empty()) {
                (*_current.back())[_fieldName.c_str()] = value.c_str();
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

        template <typename ValueT>
        void value(ValueT& value) {
            if (_fieldName && !_current.empty() && current().is_object()) {
                auto& field = current()[_fieldName.c_str()];
                if constexpr (std::is_fundamental_v<ValueT> && (std::is_integral_v<ValueT> || std::is_floating_point_v<ValueT>)) {
                    if (field.is_primitive()) {
                        value = current()[_fieldName.c_str()].get<ValueT>();
                    }
                }
                else if constexpr (std::is_same_v<up::string, ValueT>) {
                    if (field.is_primitive()) {
                        value = current()[_fieldName.c_str()].get<ValueT>();
                    }
                }
            }
        }

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
                _current.push_back(&obj);
                return Action::Enter;
            }
            return Action::Skip;
        }

        void leaveObject() noexcept {
            _current.pop_back();
        }

        template <typename T>
        void primitive(T&& value) {
            if (_fieldName && !_current.empty() && current().is_object()) {
                auto& field = current()[_fieldName.c_str()];
                if (field.is_primitive()) {
                    value = current()[_fieldName.c_str()].get<up::remove_cvref_t<T>>();
                }
            }
        }

        void string(up::string& value) {
            if (_fieldName && !_current.empty() && current().is_object()) {
                auto& field = current()[_fieldName.c_str()];
                if (field.is_string()) {
                    value = current()[_fieldName.c_str()].get<up::string>();
                }
            }
        }

        template <typename T>
        void string(T&&) {}

        nlohmann::json& current() noexcept {
            return *_current.back();
        }

        nlohmann::json& _root;
        vector<nlohmann::json*> _current;
        zstring_view _fieldName;
    };
} // namespace up::reflex
