// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "common.h"

#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class Stream;

    class FileSystem {
    public:
        FileSystem() = default;

        FileSystem(FileSystem const&) = delete;
        FileSystem& operator=(FileSystem const&) = delete;
        UP_RUNTIME_API bool fileExists(zstring_view path) const noexcept;
        UP_RUNTIME_API bool directoryExists(zstring_view path) const noexcept;

        UP_RUNTIME_API IOResult fileStat(zstring_view path, FileStat& outInfo) const;

        UP_RUNTIME_API Stream openRead(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) const;
        UP_RUNTIME_API Stream openWrite(zstring_view path, FileOpenMode mode = FileOpenMode::Binary);

        UP_RUNTIME_API EnumerateResult enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts = EnumerateOptions::None) const;

        UP_RUNTIME_API IOResult createDirectories(zstring_view path);

        UP_RUNTIME_API IOResult remove(zstring_view path);
        UP_RUNTIME_API IOResult removeRecursive(zstring_view path);

        UP_RUNTIME_API string currentWorkingDirectory() const noexcept;
        UP_RUNTIME_API bool currentWorkingDirectory(zstring_view path);

        UP_RUNTIME_API IOResult copyFileTo(zstring_view fromPath, zstring_view toPath);
        UP_RUNTIME_API IOResult moveFileTo(zstring_view fromPath, zstring_view toPath);
    };
} // namespace up
