// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "recon_messages_schema.h"

#include "potato/reflex/schema.h"
#include "potato/reflex/serialize.h"
#include "potato/runtime/io_loop.h"

#include <nlohmann/json.hpp>

namespace up {
    class ReconProtocol {
    public:
        template <typename MessageT>
        using Callback = delegate<void(MessageT const&)>;

        template <typename MessageT>
        bool send(zstring_view name, MessageT const& msg, IOPipe& pipe) {
            return _send(name, reflex::getSchema<MessageT>(), &msg, pipe);
        }

        UP_RECON_API bool receive(view<char> input);

        template <typename MessageT>
        void on(string name, Callback<MessageT> callback) {
            _handlers.push_back(new_box<Handler<MessageT>>(move(name), move(callback)));
        }

    private:
        struct HandlerBase;
        template <typename MessageT>
        struct Handler;

        enum class DecodeState {
            Ready,
            Header,
            Body,
        };

        UP_RECON_API bool _send(zstring_view name, reflex::Schema const& schema, void const* object, IOPipe& pipe);
        bool _handle(string_view message, view<char> body);

        static constexpr int headerLength = 128;

        vector<char> _buffer;
        DecodeState _state = DecodeState::Ready;
        string _headerMessageType;
        size_t _headerContentLength;
        vector<box<HandlerBase>> _handlers;
    };

    struct ReconProtocol::HandlerBase {
        virtual ~HandlerBase() = default;

        virtual bool match(string_view nm) const noexcept = 0;
        virtual bool handle(nlohmann::json const& data) = 0;
        UP_RECON_API bool decode(nlohmann::json const& data, reflex::Schema const& schema, void* object);
    };

    template <typename MessageT>
    struct ReconProtocol::Handler : HandlerBase {
        using Callback = ReconProtocol::Callback<MessageT>;

        Handler(string nm, Callback cb) : name(move(nm)), schema(reflex::getSchema<MessageT>()), callback(move(cb)) {}

        bool match(string_view nm) const noexcept override { return nm == name; }

        bool handle(nlohmann::json const& data) override {
            MessageT msg;
            if (!decode(data, schema, &msg)) {
                return false;
            }
            callback(msg);
            return true;
        }

        string name;
        reflex::Schema const& schema;
        Callback callback;
    };
} // namespace up
