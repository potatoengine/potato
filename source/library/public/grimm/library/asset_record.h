// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "common.h"
#include "grimm/foundation/vector.h"

namespace gm {
    struct AssetRecord {
        AssetId assetId;
        std::string path;
        uint64 contentHash;
    };
} // namespace gm
