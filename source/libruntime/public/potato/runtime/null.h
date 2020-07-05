// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "filesystem.h"

namespace up {
    class NullFileSystem final : public FileSystem {
    public:
        NullFileSystem() noexcept = default;

        UP_RUNTIME_API bool fileExists(zstring_view path) const noexcept override;
        UP_RUNTIME_API bool directoryExists(zstring_view path) const noexcept override;

        UP_RUNTIME_API IOResult fileStat(zstring_view path, FileStat& outInfo) const noexcept override;

        UP_RUNTIME_API Stream openRead(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) const noexcept override;
        UP_RUNTIME_API Stream openWrite(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) noexcept override;

        UP_RUNTIME_API EnumerateResult enumerate(zstring_view path,
            EnumerateCallback cb,
            EnumerateOptions opts = EnumerateOptions::None) const noexcept override;

        UP_RUNTIME_API IOResult createDirectories(zstring_view path) noexcept override;

        UP_RUNTIME_API IOResult remove(zstring_view path) noexcept override;
        UP_RUNTIME_API IOResult removeRecursive(zstring_view path) noexcept override;

        UP_RUNTIME_API string currentWorkingDirectory() const noexcept override;
        UP_RUNTIME_API bool currentWorkingDirectory(zstring_view path) override;

        UP_RUNTIME_API IOResult copyFile(zstring_view destPath, zstring_view sourcePath) override;
        UP_RUNTIME_API IOResult moveFile(zstring_view destPath, zstring_view sourcePath) override;
    };
} // namespace up
