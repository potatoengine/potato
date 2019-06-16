// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#include "_export.h"
#include "metadata.h"
#include <potato/foundation/string_writer.h>
#include <potato/foundation/string_format.h>
#include <potato/foundation/vector.h>
#include <nlohmann/json.hpp>

namespace up::reflex {
    class StreamSerializer {
    public:
        enum class Action {
            Enter,
            Skip
        };

        template <typename ClassType, typename FieldType>
        constexpr void field(zstring_view name, ClassType& object, FieldType ClassType::*field) {
            enterField(name);
            serialize(object.*field, *this);
            leaveField();
        }

        void value(int value) { primitive(value); }
        void value(float value) { primitive(value); }
        void value(string_view value) { string(value); }

    private:
        virtual Action enterField(zstring_view name) { return Action::Skip; }
        virtual void leaveField() {}

        virtual Action enterObject() { return Action::Skip; }
        virtual void leaveObject() {}

        virtual void primitive(int) {}
        virtual void primitive(float) {}
        virtual void primitive(double value) {}

        virtual void string(string_view value) {}
    }; // namespace up::reflect

    class XmlStreamSerializer : public StreamSerializer {
    public:
        constexpr XmlStreamSerializer(string_writer& writer) noexcept : _writer(writer) {}

        virtual Action enterField(zstring_view name) {
            format_into(_writer, "<field name=\"{}\">", name);
            return Action::Enter;
        }
        virtual void leaveField() {
            _writer.write("</field>");
        }

        virtual Action enterObject() {
            format_into(_writer, "<object type=\"{}\">", "object");
            return Action::Enter;
        }
        virtual void leaveObject() {
            _writer.write("</object>");
        }

        void primitive(int value) override {
            format_into(_writer, "<int>{}</int>", value);
        }

        void primitive(float value) override {
            format_into(_writer, "<float>{}</float>", value);
        }

        void primitive(double value) override {
            format_into(_writer, "<double>{}</double>", value);
        }

        void string(string_view value) override {
            format_into(_writer, "<string>{}</string>", value);
        }

    private:
        string_writer& _writer;
    };

    class JsonStreamSerializer : public StreamSerializer {
    public:
        JsonStreamSerializer(nlohmann::json& root) noexcept : _root(root), _current({&_root}) {}

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
    }; // namespace up::reflex
} // namespace up::reflex
