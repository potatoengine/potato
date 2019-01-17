// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "filesystem.h"
#include "backend.h"

namespace gm::fs {
    class NativeBackend : public Backend {
    public:
        static GM_FILESYSTEM_API FileSystem create();

        GM_FILESYSTEM_API bool fileExists(zstring_view path) const noexcept override;
        GM_FILESYSTEM_API bool directoryExists(zstring_view path) const noexcept override;

        GM_FILESYSTEM_API std::ifstream openRead(zstring_view path) const override;
        GM_FILESYSTEM_API std::ofstream openWrite(zstring_view path) override;

        GM_FILESYSTEM_API EnumerateResult enumerate(zstring_view path, EnumerateCallback& cb, EnumerateOptions opts = EnumerateOptions::None) const override;

        GM_FILESYSTEM_API bool createDirectories(zstring_view path) override;

        GM_FILESYSTEM_API bool copyFile(zstring_view from, zstring_view to) override;

    private:
        NativeBackend() = default;

        // needed as NativeBackend is the default FileSystem
        friend class FileSystem;
    };
} // namespace gm::fs
