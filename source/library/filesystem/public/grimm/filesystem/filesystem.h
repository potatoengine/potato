// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/rc.h"
#include "grimm/foundation/zstring_view.h"
#include "grimm/foundation/delegate.h"
#include "common.h"
#include "backend.h"

namespace gm::fs {
    class Backend;
    class Stream;

    class FileSystem {
    public:
        GM_FILESYSTEM_API FileSystem();
        GM_FILESYSTEM_API /*explicit*/ FileSystem(rc<Backend> backend);
        GM_FILESYSTEM_API ~FileSystem();

        static GM_FILESYSTEM_API rc<Backend> swapDefaultBackend(rc<Backend> backend);

        bool fileExists(zstring_view path) const noexcept { return _impl->fileExists(path); }
        bool directoryExists(zstring_view path) const noexcept { return _impl->directoryExists(path); }

        [[nodiscard]] Result fileStat(zstring_view path, FileStat& outInfo) const { return _impl->fileStat(path, outInfo); }

        GM_FILESYSTEM_API Stream openRead(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) const noexcept;
        GM_FILESYSTEM_API Stream openWrite(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) const noexcept;

        EnumerateResult enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts = EnumerateOptions::None) const { return _impl->enumerate(path, cb, opts); }

        [[nodiscard]] Result createDirectories(zstring_view path) { return _impl->createDirectories(path); }

        [[nodiscard]] Result copyFile(zstring_view from, zstring_view to) { return _impl->copyFile(from, to); }

        [[nodiscard]] Result remove(zstring_view path) { return _impl->remove(path); }
        [[nodiscard]] Result removeRecursive(zstring_view path) { return _impl->removeRecursive(path); }

    private:
        static rc<Backend>& activeDefaultBackend();

        rc<Backend> _impl;
    };
} // namespace gm::fs