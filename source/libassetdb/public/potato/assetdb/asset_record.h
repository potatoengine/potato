// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "common.h"
#include "potato/spud/string.h"
#include "potato/spud/span.h"
#include "potato/spud/vector.h"
#include "potato/spud/int_types.h"

namespace up {
    enum class AssetCategory : uint8 {
        Source, // lives in resources/
        Intermediate, // lives in build/cache/
        Output // lives in build/resources/
    };
    UP_ASSETDB_API span<string_view const> assetCategoryNames() noexcept;
    UP_ASSETDB_API string_view assetCategoryName(AssetCategory category) noexcept;
    UP_ASSETDB_API AssetCategory assetCategoryFromName(string_view name) noexcept;

    enum class AssetDependencyType : uint8 {
        Source,
        Runtime,
        Tool
    };
    UP_ASSETDB_API span<string_view const> assetDependencyTypeNames() noexcept;

    struct AssetDependencyRecord {
        string path;
        uint64 contentHash = 0;
    };

    struct AssetOutputRecord {
        string path;
        uint64 contentHash = 0;
    };

    struct AssetImportRecord {
        AssetId assetId = AssetId::Invalid;
        string path;
        string importerName;
        uint64 contentHash = 0;
        uint64 importerRevision = 0;
        AssetCategory category = AssetCategory::Source;

        vector<AssetDependencyRecord> sourceDependencies;
        vector<AssetOutputRecord> outputs;
    };
} // namespace up
