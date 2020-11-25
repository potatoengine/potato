// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_browser.h"
#include "editor.h"

#include "potato/editor/imgui_ext.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/path.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>
#include <imgui_internal.h>

auto up::shell::createAssetBrowser(
    ResourceLoader& resourceLoader,
    AssetBrowser::OnFileSelected onFileSelected,
    AssetBrowser::OnFileImport onFileImport) -> box<Editor> {
    return new_box<AssetBrowser>(resourceLoader, std::move(onFileSelected), std::move(onFileImport));
}

void up::shell::AssetBrowser::configure() {
    auto const foldersId = addPanel("Folders", [this] { _showFolders(); });
    dockPanel(foldersId, ImGuiDir_Left, contentId(), 0.25f);

    _rebuild();
}

void up::shell::AssetBrowser::content() {
    int depth = 0;

    ImGuiWindow* const window = ImGui::GetCurrentWindow();
    ImDrawList* const drawList = window->DrawList;

    float const availWidth = ImGui::GetContentRegionAvailWidth();
    constexpr float width = 128;
    constexpr float iconWidth = 96;
    int const columns = static_cast<int>(availWidth) / width;

    if (ImGui::BeginTable("##assets", columns)) {
        for (Asset const& asset : _assets) {
            if (asset.folderIndex != _selectedFolder) {
                continue;
            }

            ImGui::TableNextColumn();
            ImGui::PushID(asset.name.c_str());

            const char* icon = reinterpret_cast<char const*>(ICON_FA_FILE);

            ImVec2 const size = ImGui::CalcItemSize({width, width}, 0.0f, 0.0f);
            ImRect const bounds(window->DC.CursorPos, window->DC.CursorPos + size);
            ImGuiID const id = ImGui::GetID("##button");

            ImGui::ItemSize(size);
            if (ImGui::ItemAdd(bounds, id)) {
                bool hovered = false;
                bool held = false;
                bool const pressed = ImGui::ButtonBehavior(bounds, id, &hovered, &held, 0);
                if (pressed) {
                    _handleFileClick(asset.name);
                }

                if (hovered) {
                    drawList->AddRectFilled(bounds.Min, bounds.Max, ImGui::GetColorU32(ImGuiCol_FrameBgHovered));
                }

                ImVec2 const iconPos{bounds.Min.x + (width - iconWidth), bounds.Min.y};
                ImGui::SetWindowFontScale(iconWidth / ImGui::GetFontSize());
                drawList->AddText(iconPos, ImGui::GetColorU32(ImGuiCol_Text), icon);
                ImGui::SetWindowFontScale(1.f);

                ImVec2 const textPos{bounds.Min.x, bounds.Max.y - ImGui::GetFontSize()};
                drawList->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), asset.name.c_str());
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void up::shell::AssetBrowser::_showFolder(int index) {
    unsigned flags = 0;

    bool const hasChildren = _folders[index].firstChild != -1;
    if (!hasChildren) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (ImGui::TreeNodeEx(_folders[index].name.c_str(), flags)) {
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            _selectedFolder = index;
        }

        for (int childIndex = _folders[index].firstChild; childIndex != -1;
             childIndex = _folders[childIndex].nextSibling) {
            _showFolder(childIndex);
        }
        ImGui::TreePop();
    }
}

void up::shell::AssetBrowser::_showFolders() {
    if (ImGui::TreeNodeEx("<root>", _folders.empty() ? ImGuiTreeNodeFlags_Leaf : 0)) {
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            _selectedFolder = -1;
        }

        for (int index : sequence<int>(static_cast<int>(_folders.size()))) {
            if (_folders[index].depth == 0) {
                _showFolder(index);
            }
        }
        ImGui::TreePop();
    }
}

void up::shell::AssetBrowser::_rebuild() {
    ResourceManifest const& manifest = _resourceLoader.manifest();

    _folders.clear();
    _assets.clear();

    for (ResourceManifest::Record const& record : manifest.records()) {
        auto const lastSepIndex = record.filename.find_last_of("/"_sv);
        auto const start = lastSepIndex != string::npos ? lastSepIndex + 1 : 0;

        int folderIndex = -1;
        if (lastSepIndex != string::npos) {
            folderIndex = static_cast<int>(_folders.size());
            _folders.push_back({.name = string{record.filename.substr(0, lastSepIndex)}});
        }

        _assets.push_back({.name = string{record.filename.substr(start)}, .folderIndex = folderIndex});
    }
}

void up::shell::AssetBrowser::_handleFileClick(zstring_view name) {
    if (_onFileSelected != nullptr && !name.empty()) {
        _onFileSelected(name);
    }
}

void up::shell::AssetBrowser::_handleImport(zstring_view name, bool force) {
    if (_onFileImport != nullptr && !name.empty()) {
        _onFileImport(name, force);
    }
}
