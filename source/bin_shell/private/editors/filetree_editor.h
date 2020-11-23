// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "editor.h"

#include "potato/spud/delegate.h"
#include "potato/spud/hash.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>

namespace up::shell {
    class FileTreeEditor : public Editor {
    public:
        using OnFileSelected = delegate<void(zstring_view name)>;
        using OnFileImport = delegate<void(zstring_view name)>;

        explicit FileTreeEditor(string path, OnFileSelected onFileSelected, OnFileImport onFileImport)
            : Editor("FileTreeEditor"_zsv)
            , _onFileSelected(std::move(onFileSelected))
            , _onFileImport(std::move(onFileImport))
            , _path(std::move(path)) {}

        zstring_view displayName() const override { return "Files"; }
        zstring_view editorClass() const override { return "potato.editor.files"; }
        EditorId uniqueId() const override { return hash_value("/"); }

    protected:
        void configure() override {}
        void content() override;
        bool isClosable() override { return false; }

    private:
        struct Entry {
            string name;
            size_t size = 0;
            int depth = 0;
            bool directory = false;
            bool open = false;
        };

        void _enumerateFiles();
        void _handleFileClick(zstring_view name);
        void _handleImport(zstring_view name);

        OnFileSelected _onFileSelected;
        OnFileImport _onFileImport;
        string _path = "resources";
        vector<Entry> _cache;
    };

    auto createFileTreeEditor(
        string path,
        FileTreeEditor::OnFileSelected onFileSelected,
        FileTreeEditor::OnFileImport onFileImport) -> box<Editor>;
} // namespace up::shell
