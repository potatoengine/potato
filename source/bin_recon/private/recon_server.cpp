// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_server.h"

#include <nlohmann/json.hpp>
#include <iostream>

up::recon::ReconServer::ReconServer(IOLoop& loop, Logger& logger) : _logger("ReconServer", logger) {
    _source = loop.createPipeFor(0);
    _sink = loop.createPipeFor(1);

    _handler.on<schema::ReconImportMessage>("IMPORT", [this](schema::ReconImportMessage const& msg) {
        if (_importHandler) {
            _importHandler(msg);
        }
    });

    _handler.on<schema::ReconImportAllMessage>("IMPORT_ALL", [this](schema::ReconImportAllMessage const& msg) {
        if (_importAllHandler) {
            _importAllHandler(msg);
        }
    });
}

up::recon::ReconServer::~ReconServer() = default;

void up::recon::ReconServer::onImport(ImportHandler handler) {
    _importHandler = std::move(handler);
}

void up::recon::ReconServer::onImportAll(ImportAllHandler handler) {
    _importAllHandler = std::move(handler);
}

void up::recon::ReconServer::onDisconnect(DisconnectHandler handler) {
    _disconnectHandler = std::move(handler);
}

bool up::recon::ReconServer::sendLog(schema::ReconLogMessage const& msg) {
    return _send("LOG", msg);
}

bool up::recon::ReconServer::sendManifest(schema::ReconManifestMessage const& msg) {
    return _send("MANIFEST", msg);
}

bool up::recon::ReconServer::start() {
    _source.startRead([this](span<char> input) {
        if (!_handler.receive(input)) {
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
