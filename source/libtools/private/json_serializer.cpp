// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "json_serializer.h"

#include <nlohmann/json.hpp>

bool up::JsonSerializer::serializeRaw(reflex::Schema const& schema, void const* target) {
    switch (schema.primitive) {
        case reflex::SchemaPrimitive::Int32:
            _json = *static_cast<int32 const*>(target);
            return true;
        case reflex::SchemaPrimitive::Object:
            return serializeObject(schema, target);
        default:
            return false;
    }
}

bool up::JsonSerializer::serializeObject(reflex::Schema const& schema, void const* target) {
    UP_ASSERT(schema.primitive == reflex::SchemaPrimitive::Object);
    _json = nlohmann::json::object();
    for (auto const& field : schema.fields) {
        auto fieldSerializer = JsonSerializer(_json[field.name.c_str()]);
        if (!fieldSerializer.serializeRaw(*field.schema, reinterpret_cast<byte const*>(target) + field.offset)) {
            return false;
        }
    }
    return true;
}
