// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "schema.h"

#include <nlohmann/json.hpp>

namespace up::reflex {
    class JsonEncoder {
    public:
        nlohmann::json document() noexcept { return std::move(_document); }

        template <typename T>
        bool encodeObject(T const& obj) {
            Schema const& schema = getSchema<T>();
            return encodeObjectRaw(schema, &obj);
        }

        bool encodeObjectRaw(Schema const& schema, void const* obj);

    private:
        bool _encodeObject(nlohmann::json& json, Schema const& schema, void const* obj);
        bool _encodeArray(nlohmann::json& json, Schema const& schema, void const* arr);
        bool _encodeField(nlohmann::json& json, SchemaField const& field, void const* member);
        bool _encodeValue(nlohmann::json& json, Schema const& schema, void const* obj);

        nlohmann::json _document;
    };

    class JsonDecoder {
    public:
        template <typename T>
        bool decode(nlohmann::json const& json, T& obj) {
            Schema const& schema = getSchema<T>();
            return decodeRaw(json, schema, &obj);
        }

        bool decodeRaw(nlohmann::json const& json, Schema const& schema, void* obj);

    private:
        bool _decodeObject(nlohmann::json const& json, Schema const& schema, void* obj);
        bool _decodeArray(nlohmann::json const& json, Schema const& schema, void* arr);
        bool _decodeField(nlohmann::json const& json, SchemaField const& field, void* member);
        bool _decodeValue(nlohmann::json const& json, Schema const& schema, void* obj);

        nlohmann::json _document;
    };
} // namespace up::reflex
