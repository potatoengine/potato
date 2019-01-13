// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "backend.h"

namespace gm::fs {
    class NullFileSystemBackend : public FileSystemBackend {
    public:
        GM_FILESYSTEM_API NullFileSystemBackend() noexcept;
        virtual ~NullFileSystemBackend() {}

        GM_FILESYSTEM_API bool fileExists(zstring_view path) const noexcept override;
        GM_FILESYSTEM_API bool directoryExists(zstring_view path) const noexcept override;
    };
} // namespace gm::fs
