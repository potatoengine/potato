// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "filesystem.h"
#include "backend.h"

namespace up::fs {
    class NativeBackend : public Backend {
    public:
        static UP_FILESYSTEM_API FileSystem create();

        UP_FILESYSTEM_API bool fileExists(zstring_view path) const noexcept override;
        UP_FILESYSTEM_API bool directoryExists(zstring_view path) const noexcept override;

        UP_FILESYSTEM_API Result fileStat(zstring_view path, FileStat& outInfo) const override;

        UP_FILESYSTEM_API Stream openRead(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) const override;
        UP_FILESYSTEM_API Stream openWrite(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) override;

        UP_FILESYSTEM_API EnumerateResult enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts = EnumerateOptions::None) const override;

        UP_FILESYSTEM_API Result createDirectories(zstring_view path) override;

        UP_FILESYSTEM_API Result copyFile(zstring_view from, zstring_view to) override;

        UP_FILESYSTEM_API Result remove(zstring_view path) override;
        UP_FILESYSTEM_API Result removeRecursive(zstring_view path) override;

    private:
        NativeBackend() = default;

        // needed as NativeBackend is the default FileSystem
        friend class FileSystem;
    };
} // namespace up::fs
