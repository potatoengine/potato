// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/delegate_ref.h"

namespace gm::fs {
    enum Result {
        Success,
        AccessDenied,
        FileNotFound,
        System,
        UnsupportedOperation,
        Unknown,
    };

    enum EnumerateResult {
        Continue,
        Recurse,
        Break,
    };

    enum FileOpenMode {
        Binary,
        Text
    };

    enum FileType {
        Regular,
        Directory,
        SymbolicLink,
        Other
    };

    struct FileStat {
        std::size_t size = 0;
        uint64 mtime = 0;
        FileType type = FileType::Regular;
    };

    struct FileInfo {
        zstring_view path;
        std::size_t size = 0;
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

    using EnumerateCallback = gm::delegate_ref<EnumerateResult(FileInfo const&)>;
} // namespace gm::fs
