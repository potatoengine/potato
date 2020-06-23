// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "editor.h"

#include "potato/runtime/filesystem.h"
#include "potato/runtime/path.h"
#include "potato/spud/box.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>
#include <unordered_set>

namespace up::shell {
    class FileTreeEditor : public Editor {
    public:
        explicit FileTreeEditor(FileSystem& fileSystem) : Editor("FileTreeEditor"_zsv), _fileSystem(fileSystem) {}

        zstring_view displayName() const override { return "Files"; }

    protected:
        void renderContent(Renderer& renderer) override;

    private:
        struct Entry {
            string name;
            size_t size = 0;
            int depth = 0;
            bool directory = false;
            bool open = false;
        };

        void _enumerateFiles();

        FileSystem& _fileSystem;
        string _path = "resources";
        vector<Entry> _cache;
        std::unordered_set<size_t> _open;
    };

    auto createFileTreeEditor(FileSystem& fileSystem) -> box<Editor> { return new_box<FileTreeEditor>(fileSystem); }

    void FileTreeEditor::renderContent(Renderer& renderer) {
        if (_cache.empty()) {
            _enumerateFiles();
        }

        int depth = 0;
        int skipDepth = 0;

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
            if (ImGui::TreeNodeEx(cached.name.c_str(), cached.directory ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf)) {
                depth = cached.depth + 1;
                cached.open = true;
            }
            else {
                cached.open = false;
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
        _fileSystem.enumerate(_path, [this](EnumerateItem const& item) -> EnumerateResult {
            _cache.push_back({path::filename(item.info.path), item.info.size, item.depth, item.info.type == FileType::Directory});
            return item.info.type == FileType::Directory ? EnumerateResult::Recurse : EnumerateResult::Continue;
        });
    }
} // namespace up::shell
