// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/rc.h"
#include "grimm/foundation/zstring_view.h"
#include "grimm/foundation/delegate.h"
#include "common.h"
#include "backend.h"
#include <fstream>

namespace gm::fs {
    class Backend;

    class FileSystem {
    public:
        GM_FILESYSTEM_API FileSystem();
        GM_FILESYSTEM_API /*explicit*/ FileSystem(rc<Backend> backend);
        GM_FILESYSTEM_API ~FileSystem();

        static GM_FILESYSTEM_API rc<Backend> swapDefaultBackend(rc<Backend> backend);

        bool fileExists(zstring_view path) const noexcept { return _impl->fileExists(path); }
        bool directoryExists(zstring_view path) const noexcept { return _impl->directoryExists(path); }

        std::ifstream openRead(zstring_view path) const noexcept { return _impl->openRead(path); }
        std::ofstream openWrite(zstring_view path) const noexcept { return _impl->openWrite(path); }

        EnumerateResult enumerate(zstring_view path, EnumerateCallback cb) const { return _impl->enumerate(path, cb); }

        bool createDirectories(zstring_view path) { return _impl->createDirectories(path); }

        bool copyFile(zstring_view from, zstring_view to) { return _impl->copyFile(from, to); }

    private:
        static rc<Backend>& activeDefaultBackend();

        rc<Backend> _impl;
    };
} // namespace gm::fs
