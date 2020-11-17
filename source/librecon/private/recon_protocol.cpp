// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_protocol.h"

#include <nlohmann/json.hpp>

bool up::recon::encodeReconMessageRaw(nlohmann::json& target, reflex::Schema const& schema, void const* message) {
    target = nlohmann::json::object();
    target["$type"] = schema.name;

    for (reflex::SchemaField const& field : schema.fields) {
        switch (field.schema->primitive) {
            case reflex::SchemaPrimitive::String:
                target[field.name.c_str()] =
                    static_cast<string const&>(static_cast<char const*>(message) + field.offset);
                break;
            default:
                return false;
        }
    }

    return true;
}
