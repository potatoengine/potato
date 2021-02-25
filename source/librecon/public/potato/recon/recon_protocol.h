// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "recon_messages_schema.h"

#include "potato/reflex/schema.h"
#include "potato/reflex/serialize.h"

#include <nlohmann/json.hpp>

namespace up {
    template <typename MessageT, callable<MessageT> ReceiverT>
    bool decodeReconMessage(nlohmann::json const& source, ReceiverT&& receiver) {
        static reflex::Schema const& schema = reflex::getSchema<MessageT>();

        if (source["$schema"].get<string_view>() != schema.name) {
            return false;
        }

        MessageT msg;
        if (!reflex::decodeFromJson(source, msg)) {
            return false;
        }

        receiver(msg);
        return true;
    }
} // namespace up
