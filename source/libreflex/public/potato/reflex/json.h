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
        bool _encodeField(nlohmann::json& json, Schema const& schema, SchemaField const& field, void const* member);

        nlohmann::json _document;
    };
} // namespace up::reflex
