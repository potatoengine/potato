// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_edit_service.h"
#include "icons.h"

#include "potato/runtime/path.h"
#include "potato/spud/hash.h"

char8_t const* up::AssetEditService::getIconForAssetTypeHash(uint64 typeHash) const noexcept {
    if (typeHash == hash_value("potato.asset.sound")) {
        return ICON_FA_FILE_AUDIO;
    }
    if (typeHash == hash_value("potato.asset.texture")) {
        return ICON_FA_FILE_IMAGE;
    }
    if (typeHash == hash_value("potato.asset.shader")) {
        return ICON_FA_FILE_CODE;
    }
    if (typeHash == hash_value("potato.asset.model")) {
        return ICON_FA_FILE_ALT;
    }
    if (typeHash == hash_value("potato.asset.scene")) {
        return ICON_FA_FILE_VIDEO;
    }
    return ICON_FA_FILE;
}

auto up::AssetEditService::getEditorForAssetTypeHash(uint64 typeHash) const noexcept -> zstring_view {
    if (typeHash == hash_value("potato.asset.scene")) {
        return "potato.editor.scene"_zsv;
    }
    if (typeHash == hash_value("potato.asset.material")) {
        return "potato.editor.material"_zsv;
    }
    return {};
}

auto up::AssetEditService::makeFullPath(zstring_view filename) const -> string {
    return path::normalize(path::join(_assetRoot, filename), path::Separator::Native);
}
