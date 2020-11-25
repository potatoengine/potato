// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "editor.h"

#include "potato/runtime/resource_loader.h"
#include "potato/spud/delegate.h"
#include "potato/spud/hash.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>

namespace up::shell {
    class AssetBrowser : public Editor {
    public:
        using OnFileSelected = delegate<void(zstring_view name)>;
        using OnFileImport = delegate<void(zstring_view name, bool force)>;

        explicit AssetBrowser(ResourceLoader& resourceLoader, OnFileSelected onFileSelected, OnFileImport onFileImport)
            : Editor("AssetBrowser"_zsv)
            , _resourceLoader(resourceLoader)
            , _onFileSelected(std::move(onFileSelected))
            , _onFileImport(std::move(onFileImport)) {}

        zstring_view displayName() const override { return "Assets"; }
        zstring_view editorClass() const override { return "potato.editor.asset_browser"; }
        EditorId uniqueId() const override { return hash_value("/"); }

    protected:
        void configure() override;
        void content() override;
        bool isClosable() override { return false; }

    private:
        struct Folder {
            string name;
            int firstChild = -1;
            int nextSibling = -1;
            int depth = 0;
        };

        struct Asset {
            string name;
            size_t size = 0;
            int folderIndex = -1;
        };

        void _showFolder(int index);
        void _showFolders();

        void _rebuild();
        int _addFolder(string_view name, int parentIndex = 0);
        int _addFolders(string_view folders);

        void _handleFileClick(zstring_view name);
        void _handleImport(zstring_view name, bool force = false);

        ResourceLoader& _resourceLoader;
        OnFileSelected _onFileSelected;
        OnFileImport _onFileImport;
        vector<Folder> _folders;
        vector<Asset> _assets;
        int _selectedFolder = 0;
    };

    auto createAssetBrowser(
        ResourceLoader& resourceLoader,
        AssetBrowser::OnFileSelected onFileSelected,
        AssetBrowser::OnFileImport onFileImport) -> box<Editor>;
} // namespace up::shell
