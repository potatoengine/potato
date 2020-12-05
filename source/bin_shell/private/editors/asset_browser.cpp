// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_browser.h"
#include "editor.h"

#include "potato/editor/imgui_ext.h"
#include "potato/editor/imgui_fonts.h"
#include "potato/recon/recon_client.h"
#include "potato/tools/desktop.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/path.h"
#include "potato/runtime/resource_manifest.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/enumerate.h"
#include "potato/spud/numeric_util.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string.h"
#include "potato/spud/string_format.h"
#include "potato/spud/vector.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace up::shell {
    namespace {
        class AssetBrowserFactory : public EditorFactory {
        public:
            AssetBrowserFactory(
                AssetLoader& assetLoader,
                ReconClient& reconClient,
                AssetEditService& assetEditService,
                AssetBrowser::OnFileSelected onFileSelected)
                : _assetLoader(assetLoader)
                , _reconClient(reconClient)
                , _assetEditService(assetEditService)
                , _onFileSelected(std::move(onFileSelected)) {}

            zstring_view editorName() const noexcept override { return AssetBrowser::editorName; }

            box<Editor> createEditor() override {
                return new_box<AssetBrowser>(_assetLoader, _reconClient, _assetEditService, _onFileSelected);
            }

            box<Editor> createEditorForDocument(zstring_view) override { return nullptr; }

        private:
            AssetLoader& _assetLoader;
            ReconClient& _reconClient;
            AssetEditService& _assetEditService;
            AssetBrowser::OnFileSelected _onFileSelected;
        };
    } // namespace
} // namespace up::shell

auto up::shell::AssetBrowser::createFactory(
    AssetLoader& assetLoader,
    ReconClient& reconClient,
    AssetEditService& assetEditService,
    AssetBrowser::OnFileSelected onFileSelected) -> box<EditorFactory> {
    return new_box<AssetBrowserFactory>(assetLoader, reconClient, assetEditService, std::move(onFileSelected));
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

    _executeCommand();
}

void up::shell::AssetBrowser::_showAssets(int folderIndex) {
    if (ImGui::BeginIconGrid("##assets")) {
        for (Folder const& folder : _childFolders(folderIndex)) {
            _showFolder(folder);
        }

        for (Asset const& asset : _assets) {
            if (asset.folderIndex == folderIndex) {
                _showAsset(asset);
            }
        }
        ImGui::EndIconGrid();
    }
}

void up::shell::AssetBrowser::_showAsset(Asset const& asset) {
    if (ImGui::IconGridItem(
            asset.name.c_str(),
            _assetEditService.getIconForAssetTypeHash(asset.typeHash),
            _selection.selected(asset.id))) {
        _command = Command::EditAsset;
    }

    if (ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        _selection.click(
            asset.id,
            ImGui::IsModifierDown(ImGuiKeyModFlags_Ctrl),
            ImGui::IsMouseDown(ImGuiMouseButton_Right));
    }

    if (ImGui::BeginIconMenuContextPopup()) {
        if (ImGui::IconMenuItem("Edit Asset", ICON_FA_EDIT)) {
            _command = Command::EditAsset;
        }
        if (ImGui::IconMenuItem("Import", ICON_FA_DOWNLOAD)) {
            _command = Command::Import;
        }
        if (ImGui::IconMenuItem("Import (Force)")) {
            _command = Command::ForceImport;
        }
        ImGui::IconMenuSeparator();
        if (ImGui::IconMenuItem("Copy Path", ICON_FA_COPY)) {
            ImGui::SetClipboardText(asset.osPath.c_str());
        }
        if (ImGui::IconMenuItem("Copy UUID")) {
            char buf[UUID::strLength] = {0};
            format_to(buf, "{}", asset.uuid);
            ImGui::SetClipboardText(buf);
        }
        if (ImGui::IconMenuItem("Show in Explorer", ICON_FA_FOLDER_OPEN)) {
            _command = Command::OpenInExplorer;
        }
        ImGui::IconMenuSeparator();
        if (ImGui::IconMenuItem("Delete", ICON_FA_TRASH)) {
            _command = Command::Delete;
        }
        ImGui::EndPopup();
    }
}

void up::shell::AssetBrowser::_showFolder(Folder const& folder) {
    if (ImGui::IconGridItem(folder.name.c_str(), ICON_FA_FOLDER, _selection.selected(folder.id))) {
        _command = Command::OpenFolder;
    }

    if (ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        _selection.click(
            folder.id,
            ImGui::IsModifierDown(ImGuiKeyModFlags_Ctrl),
            ImGui::IsMouseDown(ImGuiMouseButton_Right));
    }

    if (ImGui::BeginIconMenuContextPopup()) {
        if (ImGui::IconMenuItem("Open", ICON_FA_FOLDER_OPEN)) {
            _command = Command::OpenFolder;
        }
        if (ImGui::IconMenuItem("Open in Explorer", nullptr)) {
            _command = Command::OpenInExplorer;
        }
        ImGui::EndPopup();
    }
}

void up::shell::AssetBrowser::_showBreadcrumb(int index) {
    if (_folders[index].parent != -1) {
        _showBreadcrumb(_folders[index].parent);
        ImGui::SameLine(0, 0);
        ImGui::TextDisabled("%s", reinterpret_cast<char const*>(ICON_FA_CARET_RIGHT));
        ImGui::SameLine(0, 0);
    }

    if (ImGui::Button(_folders[index].name.c_str())) {
        _openFolder(index);
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
    ImGui::PushID("##breadcrumbs");

    if (ImGui::IconButton("##back", ICON_FA_BACKWARD)) {
        if (_folderHistoryIndex > 0) {
            _selectedFolder = _folderHistory[--_folderHistoryIndex];
            _selection.clear();
        }
    }
    ImGui::SameLine();
    if (ImGui::IconButton("##forward", ICON_FA_FORWARD)) {
        if (_folderHistoryIndex + 1 < _folderHistory.size()) {
            _selectedFolder = _folderHistory[++_folderHistoryIndex];
            _selection.clear();
        }
    }
    ImGui::SameLine();

    _showBreadcrumb(_selectedFolder);

    ImGui::PopID();
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
            _openFolder(index);
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
    ResourceManifest const* const manifest = _assetLoader.manifest();

    _folders.clear();
    _assets.clear();

    _folders.push_back({.id = hash_value("<root>"), .name = "<root>"});

    if (manifest == nullptr) {
        return;
    }

    for (ResourceManifest::Record const& record : manifest->records()) {
        auto const lastSepIndex = record.filename.find_last_of("/"_sv);
        auto const start = lastSepIndex != string::npos ? lastSepIndex + 1 : 0;

        int folderIndex = 0;
        if (lastSepIndex != string::npos) {
            folderIndex = _addFolders(record.filename.substr(0, lastSepIndex));
        }

        _assets.push_back(
            {.id = hash_value(record.uuid),
             .uuid = record.uuid,
             .osPath = _assetEditService.makeFullPath(record.filename),
             .name = string{record.filename.substr(start)},
             .typeHash = hash_value(record.type),
             .folderIndex = folderIndex});
    }
}

int up::shell::AssetBrowser::_addFolder(string_view name, int parentIndex) {
    UP_ASSERT(parentIndex >= 0 && parentIndex < static_cast<int>(_folders.size()));

    Folder const& parent = _folders[parentIndex];

    uint64 const id = hash_combine(parent.id, hash_value(name));

    int childIndex = parent.firstChild;
    if (childIndex == -1) {
        int const newIndex = static_cast<int>(_folders.size());
        _folders.push_back(
            {.id = id, .osPath = path::join(parent.osPath, name), .name = string{name}, .parent = parentIndex});
        return _folders[parentIndex].firstChild = newIndex;
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
    _folders.push_back(
        {.id = id, .osPath = path::join(parent.osPath, name), .name = string{name}, .parent = parentIndex});
    return _folders[childIndex].nextSibling = newIndex;
}

int up::shell::AssetBrowser::_addFolders(string_view folderPath) {
    int folderIndex = 0;

    string_view::size_type sep = string_view::npos;
    while ((sep = folderPath.find('/')) != string_view::npos) {
        if (sep != 0) {
            folderIndex = _addFolder(folderPath.substr(0, sep), folderIndex);
        }
        folderPath = folderPath.substr(sep + 1);
    }

    if (!folderPath.empty()) {
        folderIndex = _addFolder(folderPath, folderIndex);
    }

    return folderIndex;
}

void up::shell::AssetBrowser::_openFolder(int index) {
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

    // clear prior selection
    _selection.clear();
}

void up::shell::AssetBrowser::_importAsset(UUID const& uuid, bool force) {
    schema::ReconImportMessage msg;
    msg.uuid = uuid;
    msg.force = force;
    _reconClient.sendMessage(msg);
}

void up::shell::AssetBrowser::_deleteAsset(UUID const& uuid) {
    schema::ReconDeleteMessage msg;
    msg.uuid = uuid;
    _reconClient.sendMessage(msg);
}

void up::shell::AssetBrowser::_executeCommand() {
    Command cmd = _command;
    _command = Command::None;

    switch (cmd) {
        case Command::OpenFolder:
            for (auto const& [index, folder] : enumerate(_folders)) {
                if (_selection.selected(folder.id)) {
                    _openFolder(index);
                    break;
                }
            }
            break;
        case Command::OpenInExplorer:
            for (Folder const& folder : _folders) {
                if (_selection.selected(folder.id)) {
                    desktop::openInExplorer(folder.osPath);
                }
            }

            {
                vector<zstring_view> files;
                for (Asset const& asset : _assets) {
                    if (_selection.selected(asset.id)) {
                        files.push_back(asset.osPath);
                    }
                }
                desktop::selectInExplorer(_assetEditService.makeFullPath(_folders[_selectedFolder].osPath), files);
            }
            break;
        case Command::EditAsset:
            for (Asset const& asset : _assets) {
                if (_selection.selected(asset.id)) {
                    _onFileSelected(asset.uuid);
                }
            }
            break;
        case Command::Delete:
            for (Asset const& asset : _assets) {
                if (_selection.selected(asset.id)) {
                    _deleteAsset(asset.uuid);
                }
            }
            break;
        case Command::Import:
        case Command::ForceImport:
            for (Asset const& asset : _assets) {
                if (_selection.selected(hash_value(asset.uuid))) {
                    _importAsset(asset.uuid, cmd == Command::ForceImport);
                }
            }
            break;
        default:
            break;
    }
}
