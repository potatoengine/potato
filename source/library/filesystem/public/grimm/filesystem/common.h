// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/delegate_ref.h"
#include "grimm/foundation/zstring_view.h"
#include "grimm/foundation/int_types.h"

namespace up::fs {
    enum class Result {
        Success,
        AccessDenied,
        FileNotFound,
        System,
        InvalidArgument,
        UnsupportedOperation,
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
        return EnumerateOptions(std::underlying_type_t<EnumerateOptions>(lhs) | std::underlying_type_t<EnumerateOptions>(rhs));
    }
    inline constexpr EnumerateOptions operator&(EnumerateOptions lhs, EnumerateOptions rhs) noexcept {
        return EnumerateOptions(std::underlying_type_t<EnumerateOptions>(lhs) & std::underlying_type_t<EnumerateOptions>(rhs));
    }

    using EnumerateCallback = up::delegate_ref<EnumerateResult(FileInfo const&)>;
} // namespace up::fs
