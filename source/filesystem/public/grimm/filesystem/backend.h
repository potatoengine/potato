// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/zstring_view.h"

namespace gm::fs {
    class FileSystemBackend {
    public:
        virtual ~FileSystemBackend() {}

        FileSystemBackend(FileSystemBackend const&) = default;
        FileSystemBackend& operator=(FileSystemBackend const&) = default;

        virtual bool fileExists(zstring_view path) const noexcept = 0;
        virtual bool directoryExists(zstring_view path) const noexcept = 0;

    protected:
        FileSystemBackend() = default;
    };
} // namespace gm::fs
