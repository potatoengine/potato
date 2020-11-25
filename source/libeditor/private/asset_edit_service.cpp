// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_edit_service.h"
#include "icons.h"

char8_t const* up::AssetEditService::getIconForType(zstring_view type) {
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
