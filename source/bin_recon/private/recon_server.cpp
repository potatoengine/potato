// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_server.h"

#include <nlohmann/json.hpp>
#include <iostream>

up::recon::ReconServer::ReconServer(IOLoop& loop, Logger& logger) : _logger("ReconServer", logger) {
    _source = loop.createPipeFor(0);
    _sink = loop.createPipeFor(1);
}

up::recon::ReconServer::~ReconServer() = default;

void up::recon::ReconServer::listenImport(ImportHandler handler) {
    _importHandler = std::move(handler);
}

void up::recon::ReconServer::listenImportAll(ImportAllHandler handler) {
    _importAllHandler = std::move(handler);
}

void up::recon::ReconServer::listenDisconnect(DisconnectHandler handler) {
    _disconnectHandler = std::move(handler);
}

bool up::recon::ReconServer::sendLog(schema::ReconLogMessage const& msg) {
    nlohmann::json doc;
    if (reflex::encodeToJson(doc, msg)) {
        return _send(doc.dump());
    }
    return false;
}

bool up::recon::ReconServer::sendManifest(schema::ReconManifestMessage const& msg) {
    nlohmann::json doc;
    if (reflex::encodeToJson(doc, msg)) {
        return _send(doc.dump());
    }
    return false;
}

bool up::recon::ReconServer::start() {
    _source.startRead([this](span<char> input) {
        auto handleImport = [&](schema::ReconImportMessage const& msg) {
            if (_importHandler) {
                _importHandler(msg);
            }
        };
        auto handleImportAll = [&](schema::ReconImportAllMessage const& msg) {
            if (_importAllHandler) {
                _importAllHandler(msg);
            }
        };

        auto doc = nlohmann::json::parse(input);
        if (!decodeReconMessage<schema::ReconImportMessage>(doc, handleImport) &&
            !decodeReconMessage<schema::ReconImportAllMessage>(doc, handleImportAll)) {
            _logger.error("Unhandled JSON message");
        }
    });
    _source.onDisconnect([this] {
        if (_disconnectHandler) {
            _disconnectHandler();
        }
    });
    return true;
}

bool up::recon::ReconServer::_send(string_view encodedMsg) {
    _sink.write(span{encodedMsg.data(), encodedMsg.size()});
    _sink.write(span{"\n", 1});
    return true;
}
