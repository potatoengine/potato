// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_edit_service.h"
#include "icons.h"

#include "potato/runtime/path.h"

char8_t const* up::AssetEditService::getIconForType(zstring_view type) const noexcept {
    if (type == "potato.asset.sound"_sv) {
        return ICON_FA_FILE_AUDIO;
    }
    if (type == "potato.asset.texture"_sv) {
        return ICON_FA_FILE_IMAGE;
    }
    if (type == "potato.asset.shader"_sv) {
        return ICON_FA_FILE_CODE;
    }
    if (type == "potato.asset.model"_sv) {
        return ICON_FA_FILE_ALT;
    }
    if (type == "potato.asset.scene"_sv) {
        return ICON_FA_FILE_VIDEO;
    }
    return ICON_FA_FILE;
}

auto up::AssetEditService::getEditorForType(zstring_view type) const noexcept -> zstring_view {
    if (type == "potato.asset.scene"_sv) {
        return "potato.editor.scene"_zsv;
    }
    return "external"_zsv;
}

auto up::AssetEditService::makeFullPath(zstring_view filename) const -> string {
    return path::normalize(path::join(_assetRoot, filename), path::Separator::Native);
}
