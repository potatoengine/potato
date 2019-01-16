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

    using EnumerateCallback = gm::delegate<EnumerateResult(FileInfo const&)>&;
} // namespace gm::fs
