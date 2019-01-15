// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/rc.h"
#include "grimm/foundation/zstring_view.h"
#include "backend.h"
#include "directory_iterator.h"
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

        DirectoryIterator recursiveEnumerate(zstring_view path) const { return _impl->recursiveEnumerate(path); }

    private:
        static rc<Backend>& activeDefaultBackend();

        rc<Backend> _impl;
    };
} // namespace gm::fs
