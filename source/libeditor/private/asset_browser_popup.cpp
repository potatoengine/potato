// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_browser_popup.h"

#include "potato/runtime/asset_loader.h"
#include "potato/spud/string_format.h"

#include <imgui.h>

bool up::assetBrowserPopup(zstring_view id, string& inout_asset, AssetLoader& assetLoader) {
    bool changed = false;

    if (ImGui::BeginPopup(id.c_str())) {
        if (ImGui::BeginTable("##assets", 4)) {
            for (ResourceManifest::Record const& asset : assetLoader.manifest().records()) {
                ImGui::TableNextColumn();
                if (ImGui::Button(asset.filename.c_str())) {
                    inout_asset = string{asset.filename};
                    changed = true;
                }
            }

            ImGui::EndTable();
        }

        ImGui::EndPopup();
    }

    return changed;
}
