// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/delegate.h"

namespace gm::fs {
    enum EnumerateResult {
        Continue,
        Recurse,
        Break,
    };

    enum FileType {
        Regular,
        Directory,
        SymbolicLink,
        Other
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

    using EnumerateCallback = gm::delegate<EnumerateResult(FileInfo const&)>;
} // namespace gm::fs
