// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "filetree_editor.h"
#include "editor.h"

#include "potato/editor/imgui_ext.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/path.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <imgui.h>

auto up::shell::createFileTreeEditor(
    string path,
    FileTreeEditor::OnFileSelected onFileSelected,
    FileTreeEditor::OnFileImport onFileImport) -> box<Editor> {
    return new_box<FileTreeEditor>(std::move(path), std::move(onFileSelected), std::move(onFileImport));
}

void up::shell::FileTreeEditor::content() {
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

        if (!cached.directory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            _handleFileClick(cached.name);
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::IconMenuItem("Open", nullptr, nullptr, false, !cached.directory)) {
                _handleFileClick(cached.name);
            }
            if (ImGui::IconMenuItem("Import", nullptr, nullptr, false, !cached.directory)) {
                _handleImport(cached.name);
            }
            ImGui::EndPopup();
        }

        ImGui::NextColumn();
        if (cached.directory) {
            ImGui::Text("-");
        }
        else {
            ImGui::Text("%lu", narrow_cast<unsigned long>(cached.size));
        }
        ImGui::NextColumn();
        auto ext = path::extension(cached.name);
        ImGui::Text("%s", cached.directory ? "<dir>" : ext.empty() ? "" : ext.substr(1).c_str());
        ImGui::NextColumn();
    }

    while (depth-- > 0) {
        ImGui::TreePop();
    }
}

void up::shell::FileTreeEditor::_enumerateFiles() {
    (void)fs::enumerate(_path, [this](auto const& item, int depth) {
        if (item.path == ".library"_zsv) {
            return fs::next;
        }

        auto ext = path::extension(item.path);
        if (ext == ".meta"_zsv) {
            return fs::recurse;
        }

        _cache.push_back({string{path::filename(item.path)}, item.size, depth, item.type == fs::FileType::Directory});
        return item.type == fs::FileType::Directory ? fs::recurse : fs::next;
    });
}

void up::shell::FileTreeEditor::_handleFileClick(zstring_view name) {
    if (_onFileSelected != nullptr && !name.empty()) {
        _onFileSelected(name);
    }
}

void up::shell::FileTreeEditor::_handleImport(zstring_view name) {
    if (_onFileImport != nullptr && !name.empty()) {
        _onFileImport(name);
    }
}
