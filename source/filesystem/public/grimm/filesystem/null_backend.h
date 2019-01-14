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

    private:
        NullBackend() = default;
    };
} // namespace gm::fs
