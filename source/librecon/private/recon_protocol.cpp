// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_protocol.h"

#include "potato/reflex/serialize.h"
#include "potato/runtime/assertion.h"

#include <nlohmann/json.hpp>

bool up::encodeReconMessageRaw(nlohmann::json& target, reflex::Schema const& schema, void const* message) {
    UP_ASSERT(message != nullptr);

    return reflex::encodeToJsonRaw(target, schema, message);
}

bool up::decodeReconMessageDispatch(nlohmann::json const& source, ReconMessageReceiverBase const& receiver) {
    using namespace schema;

    if (!source.is_object()) {
        return false;
    }

    if (!source.contains("$schema")) {
        return false;
    }

    auto const& schemaName = source["$schema"].get<string_view>();

    auto handle = [&]<typename MessageT>(tag<MessageT>) -> bool {
        static reflex::Schema const& schema = reflex::getSchema<MessageT>();
        MessageT msg{};
        if (schemaName == schema.name && reflex::decodeFromJsonRaw(source, schema, &msg)) {
            return receiver.handle(msg);
        }
        return false;
    };

    return handle(tag<ReconLogMessage>{}) || handle(tag<ReconManifestMessage>{}) || handle(tag<ReconImportMessage>{}) ||
        handle(tag<ReconImportAllMessage>{}) || handle(tag<ReconDeleteMessage>{});
}
