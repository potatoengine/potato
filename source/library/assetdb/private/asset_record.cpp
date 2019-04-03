// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/assetdb/asset_record.h"

auto up::assetCategoryNames() noexcept -> span<string_view const> {
    constexpr static string_view const names[] = {
        "Source",
        "Intermediate",
        "Output",
    };
    return names;
}

auto up::assetCategoryName(AssetCategory category) noexcept -> string_view {
    auto index = std::underlying_type_t<AssetCategory>(category);
    auto names = assetCategoryNames();
    UP_ASSERT(index >= 0 && index < names.size());
    return names[index];
}

auto up::assetCategoryFromName(string_view name) noexcept -> AssetCategory {
    auto names = assetCategoryNames();
    int index = 0;
    for (auto catName : names) {
        if (name == catName) {
            return AssetCategory(index);
        }
        ++index;
    }
    return AssetCategory::Source;
}

auto up::assetDependencyTypeNames() noexcept -> span<string_view const> {
    constexpr static string_view const names[] = {
        "Source",
        "Runtime",
        "Tool",
    };
    return names;
}
