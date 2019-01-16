// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/zstring_view.h"
#include "grimm/foundation/rc.h"
#include "common.h"
#include <fstream>

namespace gm::fs {
    class Backend : public shared<Backend> {
    public:
        virtual ~Backend() = default;

        Backend(Backend const&) = default;
        Backend& operator=(Backend const&) = default;

        virtual bool fileExists(zstring_view path) const noexcept = 0;
        virtual bool directoryExists(zstring_view path) const noexcept = 0;

        virtual std::ifstream openRead(zstring_view path) const = 0;
        virtual std::ofstream openWrite(zstring_view path) = 0;

        virtual EnumerateResult enumerate(zstring_view path, EnumerateCallback cb) const = 0;

        virtual bool createDirectories(zstring_view path) = 0;

    protected:
        Backend() = default;
    };
} // namespace gm::fs
