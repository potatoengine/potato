// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "editor.h"

#include "potato/editor/asset_edit_service.h"
#include "potato/runtime/asset_loader.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/delegate.h"
#include "potato/spud/hash.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>

namespace up::shell {
    class AssetBrowser : public Editor {
    public:
        static constexpr zstring_view editorName = "potato.editor.asset_browser"_zsv;

        using OnFileSelected = delegate<void(zstring_view name)>;
        using OnFileImport = delegate<void(zstring_view name, bool force)>;

        AssetBrowser(AssetLoader& assetLoader, OnFileSelected& onFileSelected, OnFileImport& onFileImport)
            : Editor("AssetBrowser"_zsv)
            , _assetLoader(assetLoader)
            , _onFileSelected(onFileSelected)
            , _onFileImport(onFileImport) {}

        zstring_view displayName() const override { return "Assets"; }
        zstring_view editorClass() const override { return editorName; }
        EditorId uniqueId() const override { return hash_value("/"); }

        static box<EditorFactory> createFactory(
            AssetLoader& assetLoader,
            AssetBrowser::OnFileSelected onFileSelected,
            AssetBrowser::OnFileImport onFileImport);

    protected:
        void configure() override;
        void content() override;
        bool isClosable() override { return false; }

    private:
        struct Folder {
            string name;
            int firstChild = -1;
            int nextSibling = -1;
            int parent = -1;
        };

        struct Asset {
            UUID uuid;
            AssetId logicalAssetId = AssetId::Invalid;
            string filename;
            string name;
            string type;
            size_t size = 0;
            int folderIndex = -1;
        };

        void _showAssets(int folderIndex);

        void _showBreadcrumb(int index);
        void _showBreadcrumbs();

        void _showFolder(int index);
        void _showFolders();

        void _rebuild();
        int _addFolder(string_view name, int parentIndex = 0);
        int _addFolders(string_view folders);

        void _selectFolder(int index);
        void _handleFileClick(zstring_view filename);
        void _handleImport(zstring_view name, bool force = false);

        AssetLoader& _assetLoader;
        OnFileSelected& _onFileSelected;
        OnFileImport& _onFileImport;
        AssetEditService _assetEditService;
        vector<Folder> _folders;
        vector<Asset> _assets;
        int _selectedFolder = 0;

        static constexpr int assetIconWidth = 96;

        static constexpr size_t maxFolderHistory = 64;
        size_t _folderHistoryIndex = 0;
        vector<int> _folderHistory;
    };
} // namespace up::shell
