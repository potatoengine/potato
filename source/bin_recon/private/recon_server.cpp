// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_server.h"

#include <nlohmann/json.hpp>
#include <iostream>

up::recon::ReconServer::ReconServer(Logger& logger) : _logger("ReconServer", logger) {}

up::recon::ReconServer::~ReconServer() {
    if (_ioThread.joinable()) {
        // FIXME: signal thread to terminate and wait/join
        _ioThread.detach();
    }
}

void up::recon::ReconServer::listenImport(ImportHandler handler) {
    UP_ASSERT(!_ioThread.joinable());

    _importHandler = std::move(handler);
}

void up::recon::ReconServer::listenImportAll(ImportAllHandler handler) {
    UP_ASSERT(!_ioThread.joinable());

    _importAllHandler = std::move(handler);
}

void up::recon::ReconServer::listenDisconnect(DisconnectHandler handler) {
    UP_ASSERT(!_ioThread.joinable());

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
    UP_ASSERT(!_ioThread.joinable());

    _ioThread = std::thread(_threadMain, this);
    return true;
}

void up::recon::ReconServer::_threadMain(ReconServer* server) {
    auto handleImport = [&](schema::ReconImportMessage const& msg) {
        if (server->_importHandler) {
            server->_importHandler(msg);
        }
    };
    auto handleImportAll = [&](schema::ReconImportAllMessage const& msg) {
        if (server->_importAllHandler) {
            server->_importAllHandler(msg);
        }
    };

    nlohmann::json doc;
    std::string line;
    while (std::getline(std::cin, line) && !std::cin.eof()) {
        doc = nlohmann::json::parse(line);
        if (!decodeReconMessage<schema::ReconImportMessage>(doc, handleImport) &&
            !decodeReconMessage<schema::ReconImportAllMessage>(doc, handleImportAll)) {
            server->_logger.error("Unhandled JSON message");
            break;
        }
    }

    if (server->_disconnectHandler) {
        server->_disconnectHandler();
    }
}

bool up::recon::ReconServer::_send(string_view encodedMsg) {
    std::cout << encodedMsg << "\r\n";
    std::cout.flush();
    return true;
}
