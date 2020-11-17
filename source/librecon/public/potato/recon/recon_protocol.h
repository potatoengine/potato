// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "recon_messages_schema.h"

#include "potato/reflex/schema.h"

#include <nlohmann/json_fwd.hpp>

namespace up::recon {
    bool encodeReconMessageRaw(nlohmann::json& target, reflex::Schema const& schema, void const* message);

    template <typename T>
    bool encodeReconMessage(nlohmann::json& target, T const& message) {
        return encodeReconMessageRaw(target, reflex::getSchema<T>(), &message);
    }
} // namespace up::recon
