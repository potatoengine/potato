// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "common.h"

#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class Stream;

    class FileSystem {
    public:
        virtual ~FileSystem() = default;

        FileSystem(FileSystem const&) = delete;
        FileSystem& operator=(FileSystem const&) = delete;

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

        virtual string currentWorkingDirectory() const noexcept = 0;
        virtual bool currentWorkingDirectory(zstring_view path) = 0;

    protected:
        FileSystem() = default;
    };
} // namespace up
