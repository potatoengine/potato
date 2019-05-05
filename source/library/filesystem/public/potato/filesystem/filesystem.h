// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/zstring_view.h"
#include "potato/foundation/string.h"
#include "potato/foundation/rc.h"
#include "common.h"

namespace up {
    class Stream;

    class FileSystem : public shared<FileSystem> {
    public:
        virtual ~FileSystem() = default;

        FileSystem(FileSystem const&) = default;
        FileSystem& operator=(FileSystem const&) = default;

        virtual bool fileExists(zstring_view path) const noexcept = 0;
        virtual bool directoryExists(zstring_view path) const noexcept = 0;

        virtual IOResult fileStat(zstring_view path, FileStat& outInfo) const = 0;

        virtual Stream openRead(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) const = 0;
        virtual Stream openWrite(zstring_view path, FileOpenMode mode = FileOpenMode::Binary) = 0;

        virtual EnumerateResult enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts = EnumerateOptions::None) const = 0;

        virtual IOResult createDirectories(zstring_view path) = 0;

        virtual IOResult copyFile(zstring_view from, zstring_view to) = 0;

        virtual IOResult remove(zstring_view path) = 0;
        virtual IOResult removeRecursive(zstring_view path) = 0;

        virtual auto currentWorkingDirectory() const noexcept -> string = 0;
        virtual void currentWorkingDirectory(zstring_view path) = 0;

    protected:
        FileSystem() = default;
    };
} // namespace up
