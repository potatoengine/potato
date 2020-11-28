// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_browser_popup.h"

#include "potato/runtime/asset_loader.h"
#include "potato/spud/string_format.h"

#include <imgui.h>

bool up::assetBrowserPopup(zstring_view id, ResourceId& inout_asset, string_view type, AssetLoader& assetLoader) {
    bool changed = false;

    if (ImGui::BeginPopup(id.c_str())) {
        if (ImGui::BeginTable("##assets", 4)) {
            for (ResourceManifest::Record const& asset : assetLoader.manifest().records()) {
                if (!type.empty() && type != asset.type) {
                    continue;
                }

                ImGui::TableNextColumn();
                if (ImGui::Button(asset.filename.c_str())) {
                    inout_asset = asset.logicalId;
                    changed = true;
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndTable();
        }

        ImGui::EndPopup();
    }

    return changed;
}
