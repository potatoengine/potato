// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "filesystem.h"

namespace up {
    class NativeFileSystem : public FileSystem {
    public:
        NativeFileSystem() noexcept = default;

        UP_FILESYSTEM_API bool fileExists(zstring_view path) const noexcept override;
        UP_FILESYSTEM_API bool directoryExists(zstring_view path) const noexcept override;

        UP_FILESYSTEM_API IOResult fileStat(zstring_view path, FileStat& outInfo) const override;

        UP_FILESYSTEM_API Stream openRead(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) const override;
        UP_FILESYSTEM_API Stream openWrite(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) override;

        UP_FILESYSTEM_API EnumerateResult enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts = EnumerateOptions::None) const override;

        UP_FILESYSTEM_API IOResult createDirectories(zstring_view path) override;

        UP_FILESYSTEM_API IOResult copyFile(zstring_view from, zstring_view to) override;

        UP_FILESYSTEM_API IOResult remove(zstring_view path) override;
        UP_FILESYSTEM_API IOResult removeRecursive(zstring_view path) override;

        UP_FILESYSTEM_API auto currentWorkingDirectory() const noexcept -> string;
        UP_FILESYSTEM_API bool currentWorkingDirectory(zstring_view path);
    };
} // namespace up
