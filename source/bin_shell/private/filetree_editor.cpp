// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "editor.h"

#include "potato/runtime/filesystem.h"
#include "potato/runtime/path.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>
#include <unordered_set>

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
        void renderContent(Renderer& renderer) override;
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

    auto createFileTreeEditor(string path, FileTreeEditor::OnFileSelected onFileSelected) -> box<Editor> {
        return new_box<FileTreeEditor>(std::move(path), std::move(onFileSelected));
    }

    void FileTreeEditor::renderContent(Renderer& renderer) {
        if (_cache.empty()) {
            _enumerateFiles();
        }

        int depth = 0;

        ImGui::Columns(3);
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Size");
        ImGui::NextColumn();
        ImGui::Text("Type");
        ImGui::NextColumn();

        for (auto& cached : _cache) {
            if (cached.depth > depth) {
                continue;
            }

            while (depth > cached.depth) {
                --depth;
                ImGui::TreePop();
            }

            ImGui::SetNextItemOpen(cached.open, ImGuiCond_Always);
            if (ImGui::TreeNodeEx(
                    cached.name.c_str(),
                    cached.directory ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf)) {
                depth = cached.depth + 1;
                cached.open = true;
            }
            else {
                cached.open = false;
            }

            if (!cached.directory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                _handleFileClick(cached.name);
            }

            ImGui::NextColumn();
            if (cached.directory) {
                ImGui::Text("-");
            }
            else {
                ImGui::Text("%lu", cached.size);
            }
            ImGui::NextColumn();
            auto ext = path::extension(zstring_view(cached.name));
            ImGui::Text("%s", cached.directory ? "<dir>" : ext.empty() ? "" : ext.c_str());
            ImGui::NextColumn();
        }

        while (depth-- > 0) {
            ImGui::TreePop();
        }
    }

    void FileTreeEditor::_enumerateFiles() {
        (void)fs::enumerate(_path, [this](auto const& item, int depth) {
            _cache.push_back({path::filename(item.path), item.size, depth, item.type == fs::FileType::Directory});
            return item.type == fs::FileType::Directory ? fs::recurse : fs::next;
        });
    }

    void FileTreeEditor::_handleFileClick(zstring_view name) {
        if (_onFileSelected != nullptr && !name.empty()) {
            _onFileSelected(name);
        }
    }
} // namespace up::shell
