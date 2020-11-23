// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_protocol.h"

#include "potato/reflex/serialize.h"
#include "potato/runtime/assertion.h"

#include <nlohmann/json.hpp>

bool up::recon::encodeReconMessageRaw(nlohmann::json& target, reflex::Schema const& schema, void const* message) {
    UP_ASSERT(message != nullptr);
    UP_ASSERT(schema.baseSchema == &reflex::getSchema<schema::ReconMessage>());

    return reflex::encodeToJsonRaw(target, schema, message);
}

bool up::recon::decodeReconMessage(
    nlohmann::json const& source,
    box<schema::ReconMessage>& out_msg,
    reflex::Schema const*& out_schema) {
    static reflex::Schema const& logSchema = reflex::getSchema<schema::ReconLogMessage>();
    static reflex::Schema const& importSchema = reflex::getSchema<schema::ReconImportMessage>();

    if (!source.is_object()) {
        return false;
    }

    if (!source.contains("$schema")) {
        return false;
    }

    auto const& schemaName = source["$schema"].get<string_view>();

    if (schemaName == logSchema.name) {
        out_msg = new_box<schema::ReconLogMessage>();
        out_schema = &logSchema;
        return reflex::decodeFromJsonRaw(source, logSchema, out_msg.get());
    }

    if (schemaName == importSchema.name) {
        out_msg = new_box<schema::ReconImportMessage>();
        out_schema = &importSchema;
        return reflex::decodeFromJsonRaw(source, importSchema, out_msg.get());
    }

    out_msg.reset();
    out_schema = nullptr;

    return false;
}
