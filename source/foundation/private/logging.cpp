// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.
//
// Logging system is designed to minimize the overhead esp. in debug scenarios (Where we most likely want logging), could probably be better.

#include "logging.h"
#include "vector.h"
#include <cstdio>

namespace {
    gm::vector<gm::LoggerCallback> s_Handlers;
}

void gm::addLoggerCallback(LoggerCallback handler) {
    // no need for excess space, and this isn't a hotpath function
    s_Handlers.reserve(s_Handlers.size() + 1);

    s_Handlers.push_back(std::move(handler));
}

void gm::logLine(string_view file, int line, LogSeverity severity, string_view msg) {
    using namespace std;

    //if (loc.file() && loc.line())
    //	printf("%s(%i): ", loc.file(), loc.line());
    //else if (loc.file())
    //	printf("%s: ", loc.file());
    //else if (loc.line() > 0)
    //	printf("(%i): ", loc.line());
    printf("%.*s\n", static_cast<int>(msg.size()), msg.data());

    for (auto& handler : s_Handlers) {
        handler(file, line, severity, msg);
    }
}
