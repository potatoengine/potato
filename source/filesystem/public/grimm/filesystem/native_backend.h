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

    private:
        NativeBackend() = default;

        // needed as NativeBackend is the default FileSystem
        friend class FileSystem;
    };
} // namespace gm::fs
