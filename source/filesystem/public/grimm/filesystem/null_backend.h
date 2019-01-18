// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "backend.h"
#include "filesystem.h"

namespace gm::fs {
    class NullBackend : public Backend {
    public:
        static FileSystem create() { return FileSystem(rc<NullBackend>(new NullBackend)); }

        GM_FILESYSTEM_API bool fileExists(zstring_view path) const noexcept override;
        GM_FILESYSTEM_API bool directoryExists(zstring_view path) const noexcept override;

        GM_FILESYSTEM_API std::ifstream openRead(zstring_view path) const override;
        GM_FILESYSTEM_API std::ofstream openWrite(zstring_view path) override;

        GM_FILESYSTEM_API EnumerateResult enumerate(zstring_view path, EnumerateCallback& cb, EnumerateOptions opts = EnumerateOptions::None) const override;

        GM_FILESYSTEM_API Result createDirectories(zstring_view path) override;

        GM_FILESYSTEM_API Result copyFile(zstring_view from, zstring_view to) override;

    private:
        NullBackend() = default;
    };
} // namespace gm::fs
