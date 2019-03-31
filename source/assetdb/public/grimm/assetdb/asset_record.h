// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "common.h"
#include "grimm/foundation/gmstring.h"
#include "grimm/foundation/span.h"
#include "grimm/foundation/vector.h"

namespace gm {
    enum class AssetCategory {
        Source, // lives in resources/
        Intermediate, // lives in build/cache/
        Output // lives in build/resources/
    };
    GM_ASSETDB_API span<string_view const> assetCategoryNames() noexcept;
    GM_ASSETDB_API string_view assetCategoryName(AssetCategory category) noexcept;
    GM_ASSETDB_API AssetCategory assetCategoryFromName(string_view name) noexcept;

    enum class AssetDependencyType {
        Source,
        Runtime,
        Tool
    };
    GM_ASSETDB_API span<string_view const> assetDependencyTypeNames() noexcept;

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
} // namespace gm
