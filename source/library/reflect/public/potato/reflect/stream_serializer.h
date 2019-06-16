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
    class StreamSerializer {
    public:
        enum class Action {
            Enter,
            Skip
        };

        template <typename ClassType, typename FieldType>
        constexpr void field(zstring_view name, ClassType& object, FieldType ClassType::*field) {
            if (enterField(name) == Action::Enter) {
                recurse(object.*field);
                leaveField();
            }
        }

        void value(int value) { primitive(value); }
        void value(float value) { primitive(value); }
        void value(string_view value) { string(value); }

    private:
        template <typename T>
        constexpr void recurse(T& value) {
            if constexpr (std::is_class_v<T> && !std::is_same_v<T, string_view>) {
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

    private:
        nlohmann::json& _root;
        vector<nlohmann::json*> _current;
        zstring_view _fieldName;
    };

    class JsonStreamDeserializer {
    public:
        enum class Action {
            Enter,
            Skip
        };

        JsonStreamDeserializer(nlohmann::json& root) noexcept : _root(root), _current({&_root}) {}

        template <typename ClassType, typename FieldType>
        constexpr void field(zstring_view name, ClassType& object, FieldType ClassType::*field) {
            if (enterField(name) == Action::Enter) {
                recurse(object.*field);
                leaveField();
            }
        }

        virtual Action enterField(zstring_view name) {
            _fieldName = name;
            return Action::Enter;
        }

        virtual void leaveField() {
            _fieldName = {};
        }

        virtual Action enterObject() {
            if (_fieldName && !_current.empty() && current().contains(_fieldName.c_str())) {
                auto& obj = current()[_fieldName.c_str()];
                _current.push_back(&obj);
                return Action::Enter;
            }
            return Action::Skip;
        }

        virtual void leaveObject() {
            _current.pop_back();
        }

        void value(int& value) {
            if (_fieldName && !_current.empty()) {
                value = current()[_fieldName.c_str()].get<int>();
            }
        }

        void value(float& value) {
            if (_fieldName && !_current.empty()) {
                value = current()[_fieldName.c_str()].get<float>();
            }
        }

        void value(double& value) {
            if (_fieldName && !_current.empty()) {
                value = current()[_fieldName.c_str()].get<double>();
            }
        }

        void value(string& value) {
            if (_fieldName && !_current.empty()) {
                value = current()[_fieldName.c_str()].get<up::string>();
            }
        }

    private:
        template <typename T>
        constexpr void recurse(T& value) {
            if constexpr (std::is_class_v<T> && !std::is_same_v<T, string>) {
                if (enterObject() == Action::Enter) {
                    serialize(value, *this);
                    leaveObject();
                }
            }
            else {
                serialize(value, *this);
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
