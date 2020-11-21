// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "recon_messages_schema.h"

#include "potato/reflex/schema.h"
#include "potato/spud/concepts.h"

#include <nlohmann/json_fwd.hpp>

namespace up::recon {
    bool UP_RECON_API encodeReconMessageRaw(nlohmann::json& target, reflex::Schema const& schema, void const* message);

    template <derived_from<schema::ReconMessage> T>
    bool encodeReconMessage(nlohmann::json& target, T const& message) {
        return encodeReconMessageRaw(target, reflex::getSchema<T>(), &message);
    }

    bool UP_RECON_API decodeReconMessage(
        nlohmann::json const& source,
        box<schema::ReconMessage>& out_msg,
        reflex::Schema const*& out_schema);
} // namespace up::recon
