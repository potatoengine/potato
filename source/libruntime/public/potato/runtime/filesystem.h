// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "common.h"

#include "potato/spud/delegate_ref.h"
#include "potato/spud/int_types.h"
#include "potato/spud/string.h"
#include "potato/spud/utility.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class Stream;
} // namespace up

namespace up::fs {
    enum class EnumerateResult {
        Continue,
        Recurse,
        Break,
    };

    enum class EnumerateOptions {
        None = 0,
        FullPath = 1 << 0,
    };
    inline constexpr EnumerateOptions operator|(EnumerateOptions lhs, EnumerateOptions rhs) noexcept {
        return EnumerateOptions{to_underlying(lhs) | to_underlying(rhs)};
    }
    inline constexpr EnumerateOptions operator&(EnumerateOptions lhs, EnumerateOptions rhs) noexcept {
        return EnumerateOptions{to_underlying(lhs) & to_underlying(rhs)};
    }

    enum class FileOpenMode { Binary, Text };

    enum class FileType { Regular, Directory, SymbolicLink, Other };

    struct FileStat {
        size_t size = 0;
        uint64 mtime = 0;
        FileType type = FileType::Regular;
    };

    struct FileInfo {
        zstring_view path;
        size_t size = 0;
        FileType type = FileType::Regular;
    };

    struct EnumerateItem {
        FileInfo info;
        int depth = 0;
    };

    using EnumerateCallback = up::delegate_ref<EnumerateResult(EnumerateItem const&)>;

    UP_RUNTIME_API bool fileExists(zstring_view path) noexcept;
    UP_RUNTIME_API bool directoryExists(zstring_view path) noexcept;

    UP_RUNTIME_API IOResult fileStat(zstring_view path, FileStat& outInfo);

    UP_RUNTIME_API Stream openRead(zstring_view path, FileOpenMode mode = FileOpenMode::Binary);
    UP_RUNTIME_API Stream openWrite(zstring_view path, FileOpenMode mode = FileOpenMode::Binary);

    UP_RUNTIME_API EnumerateResult enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts = EnumerateOptions::None);

    UP_RUNTIME_API IOResult createDirectories(zstring_view path);

    UP_RUNTIME_API IOResult remove(zstring_view path);
    UP_RUNTIME_API IOResult removeRecursive(zstring_view path);

    UP_RUNTIME_API string currentWorkingDirectory() noexcept;
    UP_RUNTIME_API bool currentWorkingDirectory(zstring_view path);

    UP_RUNTIME_API IOResult copyFileTo(zstring_view fromPath, zstring_view toPath);
    UP_RUNTIME_API IOResult moveFileTo(zstring_view fromPath, zstring_view toPath);
} // namespace up::fs
