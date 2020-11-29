// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_browser_popup.h"
#include "icons.h"
#include "imgui_ext.h"

#include "potato/runtime/asset_loader.h"
#include "potato/runtime/path.h"
#include "potato/runtime/resource_manifest.h"
#include "potato/spud/string_format.h"

#include <imgui.h>

bool up::assetBrowserPopup(zstring_view id, AssetId& inout_asset, string_view type, AssetLoader& assetLoader) {
    bool changed = false;

    if (assetLoader.manifest() == nullptr) {
        return false;
    }

    char filename[64] = {
        0,
    };

    ImGui::SetNextWindowSizeConstraints({300, 240}, {0, 0});
    if (ImGui::BeginPopup(id.c_str())) {
        if (ImGui::BeginIconGrid("##assets")) {
            for (ResourceManifest::Record const& asset : assetLoader.manifest()->records()) {
                if (!type.empty() && type != asset.type) {
                    continue;
                }

                format_to(filename, "{}", path::filename(asset.filename));

                if (ImGui::IconGridItem(filename, ICON_FA_FILE)) {
                    inout_asset = static_cast<AssetId>(asset.logicalId);
                    changed = true;
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndIconGrid();
        }

        ImGui::EndPopup();
    }

    return changed;
}
