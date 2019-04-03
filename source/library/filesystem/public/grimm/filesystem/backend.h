// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/zstring_view.h"
#include "grimm/foundation/rc.h"
#include "common.h"

namespace up::fs {
    class Stream;
    class Stream;

    class Backend : public shared<Backend> {
    public:
        virtual ~Backend() = default;

        Backend(Backend const&) = default;
        Backend& operator=(Backend const&) = default;

        virtual bool fileExists(zstring_view path) const noexcept = 0;
        virtual bool directoryExists(zstring_view path) const noexcept = 0;

        virtual Result fileStat(zstring_view path, FileStat& outInfo) const = 0;

        virtual Stream openRead(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) const = 0;
        virtual Stream openWrite(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) = 0;

        virtual EnumerateResult enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts = EnumerateOptions::None) const = 0;

        virtual Result createDirectories(zstring_view path) = 0;

        virtual Result copyFile(zstring_view from, zstring_view to) = 0;

        virtual Result remove(zstring_view path) = 0;
        virtual Result removeRecursive(zstring_view path) = 0;

    protected:
        Backend() = default;
    };
} // namespace up::fs
