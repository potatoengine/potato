// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "json.h"

#include "potato/runtime/assertion.h"
#include "potato/runtime/json.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

bool up::reflex::JsonEncoder::encodeObjectRaw(Schema const& schema, void const* obj) {
    UP_ASSERT(obj != nullptr);
    UP_ASSERT(schema.primitive == SchemaPrimitive::Object);

    _document = nlohmann::json::object();

    return _encodeObject(_document, schema, obj);
}

bool up::reflex::JsonEncoder::_encodeObject(nlohmann::json& json, Schema const& schema, void const* obj) {
    UP_ASSERT(schema.primitive == SchemaPrimitive::Object);

    json = nlohmann::json::object();
    json["$schema"] = schema.name;

    bool success = true;
    for (SchemaField const& field : schema.fields) {
        if (!_encodeField(json, field, static_cast<char const*>(obj) + field.offset)) {
            success = false;
        }
    }

    return success;
}

bool up::reflex::JsonEncoder::_encodeArray(nlohmann::json& json, Schema const& schema, void const* arr) {
    UP_ASSERT(schema.primitive == SchemaPrimitive::Array);

    if (schema.operations->getSize == nullptr) {
        return false;
    }
    if (schema.operations->elementAt == nullptr) {
        return false;
    }

    json = nlohmann::json::array();

    bool success = true;
    size_t const size = schema.operations->getSize(arr);
    for (size_t index = 0; index != size; ++index) {
        void const* elem = schema.operations->elementAt(const_cast<void*>(arr), index);
        nlohmann::json sub;
        if (!_encodeValue(sub, *schema.elementType, elem)) {
            success = false;
        }
        json.push_back(std::move(sub));
    }

    return success;
}

bool up::reflex::JsonEncoder::_encodeField(nlohmann::json& json, SchemaField const& field, void const* member) {
    nlohmann::json& sub = json[field.name.c_str()];
    return _encodeValue(sub, *field.schema, member);
}

bool up::reflex::JsonEncoder::_encodeValue(nlohmann::json& json, Schema const& schema, void const* obj) {
    switch (schema.primitive) {
        case SchemaPrimitive::Bool:
            json = *static_cast<bool const*>(obj);
            return true;
        case SchemaPrimitive::Int8:
            json = *static_cast<int8 const*>(obj);
            return true;
        case SchemaPrimitive::Int16:
            json = *static_cast<int16 const*>(obj);
            return true;
        case SchemaPrimitive::Int32:
            json = *static_cast<int32 const*>(obj);
            return true;
        case SchemaPrimitive::Int64:
            json = *static_cast<int64 const*>(obj);
            return true;
        case SchemaPrimitive::UInt8:
            json = *static_cast<uint8 const*>(obj);
            return true;
        case SchemaPrimitive::UInt16:
            json = *static_cast<uint16 const*>(obj);
            return true;
        case SchemaPrimitive::UInt32:
            json = *static_cast<uint32 const*>(obj);
            return true;
        case SchemaPrimitive::UInt64:
            json = *static_cast<uint64 const*>(obj);
            return true;
        case SchemaPrimitive::Vec3:
            json = nlohmann::json::array();
            json.push_back(static_cast<glm::vec3 const*>(obj)->x);
            json.push_back(static_cast<glm::vec3 const*>(obj)->y);
            json.push_back(static_cast<glm::vec3 const*>(obj)->z);
            return true;
        case SchemaPrimitive::Mat4x4:
        case SchemaPrimitive::Quat:
            return false;
        case SchemaPrimitive::Float:
            json = *static_cast<float const*>(obj);
            return true;
        case SchemaPrimitive::Double:
            json = *static_cast<double const*>(obj);
            return true;
        case SchemaPrimitive::Enum:
            // FIXME: enums of different base types/sizes
            json = enumToString(schema, *static_cast<int64 const*>(obj));
            return true;
        case SchemaPrimitive::String:
            json = *static_cast<string const*>(obj);
            return true;
        case SchemaPrimitive::Pointer:
            if (*static_cast<void const* const*>(obj) == nullptr) {
                json = nullptr;
                return true;
            }
            else {
                return _encodeValue(json, *schema.elementType, *static_cast<void const* const*>(obj));
            }
        case SchemaPrimitive::Array:
            return _encodeArray(json, schema, obj);
        case SchemaPrimitive::Object:
            return _encodeObject(json, schema, obj);
        default:
            return false;
    }
}

bool up::reflex::JsonDecoder::decodeRaw(nlohmann::json const& json, Schema const& schema, void* obj) {
    UP_ASSERT(obj != nullptr);
    return _decodeValue(json, schema, obj);
}

bool up::reflex::JsonDecoder::_decodeObject(nlohmann::json const& json, Schema const& schema, void* obj) {
    if (!json.is_object()) {
        return false;
    }

    bool success = true;
    for (SchemaField const& field : schema.fields) {
        if (json.contains(field.name.c_str())) {
            success = _decodeField(json[field.name.c_str()], field, static_cast<char*>(obj) + field.offset) && success;
        }
    }
    return success;
}

bool up::reflex::JsonDecoder::_decodeArray(nlohmann::json const& json, Schema const& schema, void* arr) {
    if (!json.is_array()) {
        return false;
    }

    if (schema.operations->resize == nullptr) {
        return false;
    }
    if (schema.operations->elementAt == nullptr) {
        return false;
    }

    schema.operations->resize(arr, json.size());

    bool success = true;
    for (size_t index = 0; index != json.size(); ++index) {
        void* el = schema.operations->elementAt(arr, index);
        success = _decodeValue(json[index], *schema.elementType, el) && success;
    }

    return success;
}

bool up::reflex::JsonDecoder::_decodeField(nlohmann::json const& json, SchemaField const& field, void* member) {
    return _decodeValue(json, *field.schema, member);
}

bool up::reflex::JsonDecoder::_decodeValue(nlohmann::json const& json, Schema const& schema, void* obj) {
    switch (schema.primitive) {
        case SchemaPrimitive::Float:
            *static_cast<float*>(obj) = json.get<float>();
            return true;
        case SchemaPrimitive::Enum:
            // FIXME: enums of different base types/sizes
            *static_cast<int64*>(obj) = enumToValue(schema, json.get<string_view>());
            return true;
        case SchemaPrimitive::String:
            *static_cast<string*>(obj) = json.get<string>();
            return true;
        case SchemaPrimitive::Array:
            return _decodeArray(json, schema, obj);
        case SchemaPrimitive::Object:
            return _decodeObject(json, schema, obj);
        default:
            return false;
    }
}
