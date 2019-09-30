// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

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

        UP_RUNTIME_API EnumerateResult enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts = EnumerateOptions::None) const noexcept override;

        UP_RUNTIME_API IOResult createDirectories(zstring_view path) noexcept override;

        UP_RUNTIME_API IOResult copyFile(zstring_view from, zstring_view to) noexcept override;

        UP_RUNTIME_API IOResult remove(zstring_view path) noexcept override;
        UP_RUNTIME_API IOResult removeRecursive(zstring_view path) noexcept override;
    };
} // namespace up
