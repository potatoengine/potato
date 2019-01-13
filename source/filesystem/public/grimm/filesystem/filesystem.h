// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/zstring_view.h"

namespace gm::fs {
    class FileSystem {
    public:
        FileSystem() noexcept = default;
        virtual ~FileSystem() {}

        GM_FILESYSTEM_API bool fileExists(zstring_view path) const noexcept;
        GM_FILESYSTEM_API bool directoryExists(zstring_view path) const noexcept;
    };
} // namespace gm::fs
