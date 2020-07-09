// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "editor.h"

#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>

namespace up::shell {
    class FileTreeEditor : public Editor {
    public:
        using OnFileSelected = delegate<void(zstring_view name)>;

        explicit FileTreeEditor(string path, OnFileSelected onFileSelected)
            : Editor("FileTreeEditor"_zsv)
            , _onFileSelected(std::move(onFileSelected))
            , _path(std::move(path)) {}

        zstring_view displayName() const override { return "Files"; }

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

        OnFileSelected _onFileSelected;
        string _path = "resources";
        vector<Entry> _cache;
    };

    auto createFileTreeEditor(string path, FileTreeEditor::OnFileSelected onFileSelected) -> box<Editor>;
} // namespace up::shell
