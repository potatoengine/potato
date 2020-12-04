// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "recon_messages_schema.h"

#include "potato/reflex/schema.h"

#include <nlohmann/json_fwd.hpp>

namespace up {
    bool UP_RECON_API encodeReconMessageRaw(nlohmann::json& target, reflex::Schema const& schema, void const* message);

    template <typename MessageT>
    bool encodeReconMessage(nlohmann::json& target, MessageT const& message) {
        return encodeReconMessageRaw(target, reflex::getSchema<MessageT>(), &message);
    }

    class ReconMessageReceiverBase {
    public:
        virtual bool handle(schema::ReconLogMessage const& msg) const { return _handleUnknown(msg); }
        virtual bool handle(schema::ReconManifestMessage const& msg) const { return _handleUnknown(msg); }
        virtual bool handle(schema::ReconImportMessage const& msg) const { return _handleUnknown(msg); }
        virtual bool handle(schema::ReconImportAllMessage const& msg) const { return _handleUnknown(msg); }
        virtual bool handle(schema::ReconDeleteMessage const& msg) const { return _handleUnknown(msg); }

        virtual bool handleUnknownRaw(reflex::Schema const& schema, void const* object) const { return false; }

    protected:
        virtual ~ReconMessageReceiverBase() = default;

    private:
        template <typename MessageT>
        bool _handleUnknown(MessageT const& msg) const {
            return handleUnknownRaw(reflex::getSchema<MessageT>(), &msg);
        }
    };

    template <typename ReceiverT, typename MessageT>
    concept ReconReceiverHandles = requires(ReceiverT& receiver, MessageT const& msg) {
        receiver(msg);
    };

    template <typename ReceiverT>
    class ReconMessageReceiverWrapper final : public ReconMessageReceiverBase {
    public:
        ReconMessageReceiverWrapper(ReceiverT& receiver) : _receiver(receiver) {}

        bool handle(schema::ReconLogMessage const& msg) const override { return _handle(msg); }
        bool handle(schema::ReconManifestMessage const& msg) const override { return _handle(msg); }
        bool handle(schema::ReconImportMessage const& msg) const override { return _handle(msg); }
        bool handle(schema::ReconImportAllMessage const& msg) const override { return _handle(msg); }
        bool handle(schema::ReconDeleteMessage const& msg) const override { return _handle(msg); }

    private:
        template <typename MessageT>
        bool _handle(MessageT const& msg) const {
            if constexpr (ReconReceiverHandles<ReceiverT, MessageT>) {
                _receiver(msg);
                return true;
            }
            return false;
        }

        ReceiverT& _receiver;
    };

    bool UP_RECON_API decodeReconMessage(nlohmann::json const& source, ReconMessageReceiverBase const& receiver);

    template <typename ReceiverT>
    bool decodeReconMessage(nlohmann::json const& source, ReceiverT& receiver) {
        return decodeReconMessage(source, ReconMessageReceiverWrapper{receiver});
    }
} // namespace up
