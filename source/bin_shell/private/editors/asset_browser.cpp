// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_browser.h"
#include "editor.h"

#include "potato/editor/imgui_ext.h"
#include "potato/editor/imgui_fonts.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/path.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/numeric_util.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace up::shell {
    namespace {
        class AssetBrowserFactory : public EditorFactory {
        public:
            AssetBrowserFactory(
                ResourceLoader& resourceLoader,
                AssetBrowser::OnFileSelected onFileSelected,
                AssetBrowser::OnFileImport onFileImport)
                : _resourceLoader(resourceLoader)
                , _onFileSelected(std::move(onFileSelected))
                , _onFileImport(std::move(onFileImport)) {}

            zstring_view editorName() const noexcept override { return AssetBrowser::editorName; }

            box<Editor> createEditor() override {
                return new_box<AssetBrowser>(_resourceLoader, _onFileSelected, _onFileImport);
            }

            box<Editor> createEditorForAsset(zstring_view) override { return nullptr; }

        private:
            ResourceLoader& _resourceLoader;
            AssetBrowser::OnFileSelected _onFileSelected;
            AssetBrowser::OnFileImport _onFileImport;
        };
    } // namespace
} // namespace up::shell

auto up::shell::AssetBrowser::createFactory(
    ResourceLoader& resourceLoader,
    AssetBrowser::OnFileSelected onFileSelected,
    AssetBrowser::OnFileImport onFileImport) -> box<EditorFactory> {
    return new_box<AssetBrowserFactory>(resourceLoader, std::move(onFileSelected), std::move(onFileImport));
}

void up::shell::AssetBrowser::configure() {
    _rebuild();
}

void up::shell::AssetBrowser::content() {
    if (ImGui::BeginTable(
            "##asset_browser",
            2,
            ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV,
            ImGui::GetContentRegionAvail())) {
        ImGui::TableSetupColumn("##files", 0, 1);
        ImGui::TableSetupColumn("##assets", 0, 4);

        ImGui::TableNextColumn();
        _showFolders();

        ImGui::TableNextColumn();
        _showBreadcrumbs();
        _showAssets(_selectedFolder);

        ImGui::EndTable();
    }
}

void up::shell::AssetBrowser::_showAssets(int folderIndex) {
    ImGuiWindow* const window = ImGui::GetCurrentWindow();
    ImDrawList* const drawList = window->DrawList;

    float const availWidth = ImGui::GetContentRegionAvailWidth();
    constexpr float width = 128;
    constexpr float iconWidth = 96;
    int const columns = clamp(static_cast<int>(availWidth / width), 1, 64);

    if (ImGui::BeginTable("##assets", columns)) {
        for (Asset const& asset : _assets) {
            if (asset.folderIndex != folderIndex) {
                continue;
            }

            ImGui::TableNextColumn();
            ImGui::PushID(asset.name.c_str());

            const char* const icon = reinterpret_cast<char const*>(_assetEditService.getIconForType(asset.type));

            ImVec2 const size = ImGui::CalcItemSize({width, width}, 0.0f, 0.0f);
            ImRect const bounds{window->DC.CursorPos, window->DC.CursorPos + size};
            ImRect const innerBounds{bounds.Min + ImGui::GetItemSpacing(), bounds.Max - ImGui::GetItemSpacing()};
            ImGuiID const id = ImGui::GetID("##button");

            ImGui::ItemSize(size);
            if (ImGui::ItemAdd(bounds, id)) {
                bool hovered = false;
                bool held = false;
                bool const pressed = ImGui::ButtonBehavior(bounds, id, &hovered, &held, 0);
                if (pressed) {
                    _handleFileClick(asset.filename);
                }

                ImU32 const textColor = ImGui::GetColorU32(ImGuiCol_Text);
                ImU32 const bgColor = ImGui::GetColorU32(
                    held ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);

                if (hovered) {
                    drawList->AddRectFilled(bounds.Min, bounds.Max, bgColor, 8.f);
                }

                ImGui::PushFont(ImGui::UpFont::FontAwesome_96);
                ImVec2 const iconSize = ImGui::CalcTextSize(icon);
                ImVec2 const iconPos{
                    innerBounds.Min.x + (innerBounds.GetWidth() - iconSize.x) * 0.5f,
                    innerBounds.Min.y};
                drawList->AddText(iconPos, textColor, icon);
                ImGui::PopFont();

                ImVec2 const textPos{innerBounds.Min.x, innerBounds.Max.y - ImGui::GetTextLineHeightWithSpacing()};
                ImGui::TextCentered(textPos, innerBounds.Max, textColor, asset.name.c_str());
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void up::shell::AssetBrowser::_showTrail(int index) {
    if (_folders[index].parent != -1) {
        _showTrail(_folders[index].parent);
        ImGui::SameLine();
        ImGui::TextDisabled("/");
        ImGui::SameLine();
    }

    if (ImGui::Button(_folders[index].name.c_str())) {
        _selectFolder(index);
    }
    ImGui::SameLine();
}

void up::shell::AssetBrowser::_showBreadcrumbs() {
    ImGuiWindow* const window = ImGui::GetCurrentWindow();
    ImDrawList* const drawList = window->DrawList;

    drawList->AddRectFilled(
        window->DC.CursorPos,
        window->DC.CursorPos +
            ImVec2{
                ImGui::GetContentRegionAvail().x,
                ImGui::GetTextLineHeightWithSpacing() + ImGui::GetItemSpacing().y * 3},
        ImGui::GetColorU32(ImGuiCol_Header),
        4.f);

    window->DC.CursorPos += ImGui::GetItemSpacing();
    ImGui::BeginGroup();

    if (ImGui::IconButton("##back", ICON_FA_BACKWARD)) {
        if (_folderHistoryIndex > 0) {
            _selectedFolder = _folderHistory[--_folderHistoryIndex];
        }
    }
    ImGui::SameLine();
    if (ImGui::IconButton("##forward", ICON_FA_FORWARD)) {
        if (_folderHistoryIndex + 1 < _folderHistory.size()) {
            _selectedFolder = _folderHistory[++_folderHistoryIndex];
        }
    }
    ImGui::SameLine();

    _showTrail(_selectedFolder);

    ImGui::EndGroup();
    window->DC.CursorPos.y += ImGui::GetItemSpacing().y;
}

void up::shell::AssetBrowser::_showFolder(int index) {
    unsigned flags = 0;

    bool const hasChildren = _folders[index].firstChild != -1;
    if (!hasChildren) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (index == _selectedFolder) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (index == 0) {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    if (ImGui::TreeNodeEx(_folders[index].name.c_str(), flags)) {
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            _selectFolder(index);
        }

        for (int childIndex = _folders[index].firstChild; childIndex != -1;
             childIndex = _folders[childIndex].nextSibling) {
            _showFolder(childIndex);
        }
        ImGui::TreePop();
    }
}

void up::shell::AssetBrowser::_showFolders() {
    _showFolder(0);
}

void up::shell::AssetBrowser::_rebuild() {
    ResourceManifest const& manifest = _resourceLoader.manifest();

    _folders.clear();
    _assets.clear();

    _folders.push_back({.name = "<root>"});

    for (ResourceManifest::Record const& record : manifest.records()) {
        auto const lastSepIndex = record.filename.find_last_of("/"_sv);
        auto const start = lastSepIndex != string::npos ? lastSepIndex + 1 : 0;

        int folderIndex = 0;
        if (lastSepIndex != string::npos) {
            folderIndex = _addFolders(record.filename.substr(0, lastSepIndex));
        }

        _assets.push_back(
            {.filename = string{record.filename},
             .name = string{record.filename.substr(start)},
             .type = string{record.type},
             .folderIndex = folderIndex});
    }
}

int up::shell::AssetBrowser::_addFolder(string_view name, int parentIndex) {
    UP_ASSERT(parentIndex >= 0 && parentIndex < static_cast<int>(_folders.size()));

    int childIndex = _folders[parentIndex].firstChild;
    if (childIndex == -1) {
        int const newIndex = static_cast<int>(_folders.size());
        _folders.push_back({.name = string{name}, .parent = parentIndex});
        _folders[parentIndex].firstChild = newIndex;
        return newIndex;
    }

    while (_folders[childIndex].nextSibling != -1) {
        if (_folders[childIndex].name == name) {
            return childIndex;
        }
        childIndex = _folders[childIndex].nextSibling;
    }

    if (_folders[childIndex].name == name) {
        return childIndex;
    }

    int const newIndex = static_cast<int>(_folders.size());
    _folders.push_back({.name = string{name}, .parent = parentIndex});
    _folders[childIndex].nextSibling = newIndex;
    return newIndex;
}

int up::shell::AssetBrowser::_addFolders(string_view folders) {
    int folderIndex = 0;

    string_view::size_type sep;
    while ((sep = folders.find('/')) != string_view::npos) {
        if (sep != 0) {
            folderIndex = _addFolder(folders.substr(0, sep), folderIndex);
        }
        folders = folders.substr(sep + 1);
    }

    if (!folders.empty()) {
        folderIndex = _addFolder(folders, folderIndex);
    }

    return folderIndex;
}

void up::shell::AssetBrowser::_selectFolder(int index) {
    // cut any of the "future" history
    if (_folderHistory.size() > _folderHistoryIndex + 1) {
        _folderHistory.resize(_folderHistoryIndex + 1);
    }

    // ensure our history has a limited length
    if (_folderHistory.size() >= maxFolderHistory) {
        _folderHistory.erase(_folderHistory.begin());
    }

    // add history item
    _folderHistoryIndex = _folderHistory.size();
    _folderHistory.push_back(index);

    // select new folder
    _selectedFolder = index;
}

void up::shell::AssetBrowser::_handleFileClick(zstring_view filename) {
    if (_onFileSelected != nullptr && !filename.empty()) {
        _onFileSelected(filename);
    }
}

void up::shell::AssetBrowser::_handleImport(zstring_view name, bool force) {
    if (_onFileImport != nullptr && !name.empty()) {
        _onFileImport(name, force);
    }
}
