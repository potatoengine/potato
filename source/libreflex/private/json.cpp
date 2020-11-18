// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "json.h"

#include "potato/runtime/assertion.h"
#include "potato/runtime/json.h"

bool up::reflex::JsonEncoder::encodeObjectRaw(Schema const& schema, void const* obj) {
    UP_ASSERT(obj != nullptr);
    UP_ASSERT(schema.primitive == SchemaPrimitive::Object);

    _document = nlohmann::json::object();

    return _encodeObject(_document, schema, obj);
}

bool up::reflex::JsonEncoder::_encodeObject(nlohmann::json& json, Schema const& schema, void const* obj) {
    UP_ASSERT(schema.primitive == SchemaPrimitive::Object);

    json["$schema"] = schema.name;

    bool success = true;
    for (SchemaField const& field : schema.fields) {
        if (!_encodeField(json, schema, field, static_cast<char const*>(obj) + field.offset)) {
            success = false;
        }
    }

    return success;
}

bool up::reflex::JsonEncoder::_encodeField(
    nlohmann::json& json,
    Schema const& schema,
    SchemaField const& field,
    void const* member) {
    switch (field.schema->primitive) {
        case SchemaPrimitive::Enum:
            json[field.name.c_str()] = enumToString(*field.schema, *static_cast<int64 const*>(member));
            return true;
        case SchemaPrimitive::String:
            json[field.name.c_str()] = *static_cast<string const*>(member);
            return true;
        case SchemaPrimitive::Object: {
            nlohmann::json sub = nlohmann::json::object();
            bool success = _encodeObject(sub, *field.schema, member);
            json[field.name.c_str()] = std::move(sub);
            return success;
        }
        default:
            return false;
    }
}
