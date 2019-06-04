// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#include "_export.h"
#include <potato/foundation/string_writer.h>
#include <potato/foundation/string_format.h>

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
} // namespace up::reflect
