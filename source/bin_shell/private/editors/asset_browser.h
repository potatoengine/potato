// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "editor.h"
#include "selection.h"

#include "potato/editor/asset_edit_service.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/delegate.h"
#include "potato/spud/generator.h"
#include "potato/spud/hash.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>

namespace up {
    class ReconClient;
}

namespace up::shell {
    class AssetBrowser : public Editor {
    public:
        static constexpr zstring_view editorName = "potato.editor.asset_browser"_zsv;

        using OnFileSelected = delegate<void(UUID const& uuid)>;

        AssetBrowser(
            AssetLoader& assetLoader,
            ReconClient& reconClient,
            AssetEditService& assetEditService,
            OnFileSelected& onFileSelected)
            : Editor("AssetBrowser"_zsv)
            , _assetLoader(assetLoader)
            , _assetEditService(assetEditService)
            , _reconClient(reconClient)
            , _onFileSelected(onFileSelected) {}

        zstring_view displayName() const override { return "Assets"; }
        zstring_view editorClass() const override { return editorName; }
        EditorId uniqueId() const override { return hash_value("/"); }

        static box<EditorFactory> createFactory(
            AssetLoader& assetLoader,
            ReconClient& reconClient,
            AssetEditService& assetEditService,
            AssetBrowser::OnFileSelected onFileSelected);

    protected:
        void configure() override;
        void content() override;
        bool isClosable() override { return false; }

    private:
        struct Folder {
            uint64 id = 0;
            string osPath;
            string name;
            int firstChild = -1;
            int nextSibling = -1;
            int parent = -1;
        };

        struct Asset {
            uint64 id = 0;
            UUID uuid;
            string osPath;
            string name;
            uint64 typeHash;
            size_t size = 0;
            int folderIndex = -1;
        };

        enum class Command { None, OpenFolder, OpenInExplorer, EditAsset, Trash, Import, ForceImport };

        void _showAssets(int folderIndex);
        void _showAsset(Asset const& asset);
        void _showFolder(Folder const& folder);

        void _showBreadcrumb(int index);
        void _showBreadcrumbs();

        void _showTreeFolder(int index);
        void _showTreeFolders();

        void _rebuild();
        int _addFolder(string_view name, int parentIndex = 0);
        int _addFolders(string_view folderPath);

        void _openFolder(int index);
        void _importAsset(UUID const& uuid, bool force = false);

        void _executeCommand();

        generator<Folder const&> _childFolders(int folderIndex) const {
            for (int childIndex = _folders[folderIndex].firstChild; childIndex != -1;
                 childIndex = _folders[childIndex].nextSibling) {
                co_yield _folders[childIndex];
            }
        }

        AssetLoader& _assetLoader;
        ReconClient& _reconClient;
        OnFileSelected& _onFileSelected;
        AssetEditService& _assetEditService;
        SelectionState _selection;
        vector<Folder> _folders;
        vector<Asset> _assets;
        int _selectedFolder = 0;
        int _manifestRevision = 0;
        Command _command = Command::None;

        static constexpr int assetIconWidth = 96;

        static constexpr size_t maxFolderHistory = 64;
        size_t _folderHistoryIndex = 0;
        vector<int> _folderHistory{0};
    };
} // namespace up::shell
