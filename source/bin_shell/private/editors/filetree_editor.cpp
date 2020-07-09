// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "filetree_editor.h"
#include "editor.h"

#include "potato/runtime/filesystem.h"
#include "potato/runtime/path.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>

auto up::shell::createFileTreeEditor(string path, FileTreeEditor::OnFileSelected onFileSelected) -> box<Editor> {
    return new_box<FileTreeEditor>(std::move(path), std::move(onFileSelected));
}

void up::shell::FileTreeEditor::renderContent(Renderer& renderer) {
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

void up::shell::FileTreeEditor::_enumerateFiles() {
    (void)fs::enumerate(_path, [this](auto const& item, int depth) {
        _cache.push_back({path::filename(item.path), item.size, depth, item.type == fs::FileType::Directory});
        return item.type == fs::FileType::Directory ? fs::recurse : fs::next;
    });
}

void up::shell::FileTreeEditor::_handleFileClick(zstring_view name) {
    if (_onFileSelected != nullptr && !name.empty()) {
        _onFileSelected(name);
    }
}
