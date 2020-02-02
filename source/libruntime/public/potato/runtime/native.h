// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "filesystem.h"

namespace up {
    class NativeFileSystem : public FileSystem {
    public:
        NativeFileSystem() noexcept = default;

        UP_RUNTIME_API bool fileExists(zstring_view path) const noexcept override;
        UP_RUNTIME_API bool directoryExists(zstring_view path) const noexcept override;

        UP_RUNTIME_API IOResult fileStat(zstring_view path, FileStat& outInfo) const override;

        UP_RUNTIME_API Stream openRead(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) const override;
        UP_RUNTIME_API Stream openWrite(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) override;

        UP_RUNTIME_API EnumerateResult enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts = EnumerateOptions::None) const override;

        UP_RUNTIME_API IOResult createDirectories(zstring_view path) override;

        UP_RUNTIME_API IOResult copyFile(zstring_view from, zstring_view to) override;

        UP_RUNTIME_API IOResult remove(zstring_view path) override;
        UP_RUNTIME_API IOResult removeRecursive(zstring_view path) override;

        UP_RUNTIME_API string currentWorkingDirectory() const noexcept override;
        UP_RUNTIME_API bool currentWorkingDirectory(zstring_view path) override;
    };
} // namespace up
