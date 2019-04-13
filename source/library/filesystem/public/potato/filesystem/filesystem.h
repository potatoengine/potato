// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/zstring_view.h"
#include "potato/foundation/delegate.h"
#include "common.h"
#include "backend.h"

namespace up {
    class Backend;
    class Stream;

    class FileSystem {
    public:
        UP_FILESYSTEM_API FileSystem();
        UP_FILESYSTEM_API /*explicit*/ FileSystem(rc<Backend> backend);
        UP_FILESYSTEM_API ~FileSystem();

        static UP_FILESYSTEM_API rc<Backend> swapDefaultBackend(rc<Backend> backend);

        bool fileExists(zstring_view path) const noexcept { return _impl->fileExists(path); }
        bool directoryExists(zstring_view path) const noexcept { return _impl->directoryExists(path); }

        [[nodiscard]] IOResult fileStat(zstring_view path, FileStat& outInfo) const { return _impl->fileStat(path, outInfo); }

        UP_FILESYSTEM_API Stream openRead(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) const noexcept;
        UP_FILESYSTEM_API Stream openWrite(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) const noexcept;

        EnumerateResult enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts = EnumerateOptions::None) const { return _impl->enumerate(path, cb, opts); }

        [[nodiscard]] IOResult createDirectories(zstring_view path) { return _impl->createDirectories(path); }

        [[nodiscard]] IOResult copyFile(zstring_view from, zstring_view to) { return _impl->copyFile(from, to); }

        [[nodiscard]] IOResult remove(zstring_view path) { return _impl->remove(path); }
        [[nodiscard]] IOResult removeRecursive(zstring_view path) { return _impl->removeRecursive(path); }

    private:
        static rc<Backend>& activeDefaultBackend();

        rc<Backend> _impl;
    };
} // namespace up
