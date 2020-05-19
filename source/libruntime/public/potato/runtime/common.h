// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/delegate_ref.h"
#include "potato/spud/zstring_view.h"
#include "potato/spud/int_types.h"
#include "potato/spud/utility.h"

namespace up {
    enum class IOResult {
        Success,
        AccessDenied,
        FileNotFound,
        System,
        InvalidArgument,
        UnsupportedOperation,
        Malformed,
        Unknown,
    };

    enum class EnumerateResult {
        Continue,
        Recurse,
        Break,
    };

    enum class FileOpenMode {
        Binary,
        Text
    };

    enum class FileType {
        Regular,
        Directory,
        SymbolicLink,
        Other
    };

    enum class SeekPosition {
        Begin,
        End,
        Current
    };

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

    using EnumerateCallback = up::delegate_ref<EnumerateResult(FileInfo const&)>;
} // namespace up
