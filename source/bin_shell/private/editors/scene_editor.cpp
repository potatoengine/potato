// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "scene_editor.h"
#include "camera.h"
#include "camera_controller.h"
#include "editor.h"
#include "scene.h"
#include "scene_doc.h"
#include "selection.h"

#include "potato/editor/imgui_ext.h"
#include "potato/render/camera.h"
#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/renderer.h"
#include "potato/spud/delegate.h"
#include "potato/spud/fixed_string_writer.h"

#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>

auto up::shell::createSceneEditor(
    box<SceneDocument> sceneDoc,
    SceneEditor::EnumerateComponents components,
    SceneEditor::HandlePlayClicked onPlayClicked) -> box<Editor> {
    return new_box<SceneEditor>(std::move(sceneDoc), std::move(components), std::move(onPlayClicked));
}

void up::shell::SceneEditor::tick(float deltaTime) {
    _doc->scene()->tick(deltaTime);
    _doc->scene()->flush();
}

void up::shell::SceneEditor::configure() {
    auto const inspectorId = addPanel("Inspector", [this] { _inspector(); });
    auto const hierarchyId = addPanel("Hierarchy", [this] { _hierarchy(); });

    dockPanel(inspectorId, ImGuiDir_Right, contentId(), 0.25f);
    dockPanel(hierarchyId, ImGuiDir_Down, inspectorId, 0.65f);

    addAction(
        {.name = "potato.editors.scene.actions.play",
         .command = "Play Scene",
         .menu = "Actions\\Play",
         .enabled = [this] { return isActive(); },
         .action =
             [this]() {
                 _onPlayClicked(_doc->scene());
             }});
    addAction(
        {.name = "potato.editors.scene.options.grid.toggle",
         .command = "Toggle Grid",
         .menu = "View\\Options\\Grid",
         .enabled = [this] { return isActive(); },
         .checked = [this] { return _enableGrid; },
         .action =
             [this]() {
                 _enableGrid = !_enableGrid;
             }});
}

void up::shell::SceneEditor::content() {
    auto& io = ImGui::GetIO();

    auto const contentSize = ImGui::GetContentRegionAvail();

    if (contentSize.x <= 0 || contentSize.y <= 0) {
        return;
    }

    ImGui::BeginChild("SceneContent", contentSize, false);
    {
        _sceneDimensions = {contentSize.x, contentSize.y};

        glm::vec3 movement = {0, 0, 0};
        glm::vec3 motion = {0, 0, 0};

        auto const pos = ImGui::GetCursorScreenPos();
        if (_bufferView != nullptr) {
            ImGui::Image(_bufferView.get(), contentSize);
        }

        ImRect area{pos, pos + contentSize};

        auto const id = ImGui::GetID("SceneControl");
        ImGui::ItemAdd(area, id);
        ImGui::ButtonBehavior(
            area,
            id,
            nullptr,
            nullptr,
            (int)ImGuiButtonFlags_PressedOnClick | (int)ImGuiButtonFlags_MouseButtonRight |
                (int)ImGuiButtonFlags_MouseButtonMiddle);
        if (ImGui::IsItemActive()) {
            ImGui::CaptureMouseFromApp();

            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                motion.x = io.MouseDelta.x / contentSize.x * 2;
                motion.y = io.MouseDelta.y / contentSize.y * 2;
            }
            else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
                movement.x = io.MouseDelta.x;
                movement.y = io.MouseDelta.y;
            }
        }

        if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered()) {
            motion.z = io.MouseWheel > 0.f ? 1.f : io.MouseWheel < 0 ? -1.f : 0.f;
        }

        _cameraController.apply(_camera, movement, motion, io.DeltaTime);

        ImGui::EndChild();
    }
}

void up::shell::SceneEditor::render(Renderer& renderer, float deltaTime) {
    if (_sceneDimensions.x == 0 || _sceneDimensions.y == 0) {
        return;
    }

    glm::ivec2 bufferSize = _buffer != nullptr ? _buffer->dimensions() : glm::vec2{0, 0};
    if (bufferSize.x != _sceneDimensions.x || bufferSize.y != _sceneDimensions.y) {
        _resize(renderer.device(), _sceneDimensions);
    }

    if (_renderCamera == nullptr) {
        _renderCamera = new_box<RenderCamera>();
    }

    if (_buffer != nullptr) {
        renderer.beginFrame();
        auto ctx = renderer.context();

        _renderCamera->resetBackBuffer(_buffer);
        if (_enableGrid) {
            _drawGrid();
        }
        _renderCamera->beginFrame(ctx, _camera.position(), _camera.matrix());
        if (_doc->scene() != nullptr) {
            _doc->scene()->render(ctx);
        }
        renderer.flushDebugDraw(deltaTime);
        renderer.endFrame(deltaTime);
    }
}

void up::shell::SceneEditor::_drawGrid() {
    auto constexpr guidelines = 10;

    // The real intent here is to keep the grid roughly the same spacing in
    // pixels on the screen; this doesn't really accomplish that, though.
    // Improvements welcome.
    //
    auto const cameraPos = _camera.position();
    auto const logDist = std::log2(std::abs(cameraPos.y));
    auto const spacing = std::max(1, static_cast<int>(logDist) - 3);

    auto const guideSpacing = static_cast<float>(guidelines * spacing);
    float x = std::trunc(cameraPos.x / guideSpacing) * guideSpacing;
    float z = std::trunc(cameraPos.z / guideSpacing) * guideSpacing;

    DebugDrawGrid grid;
    grid.axis2 = {0, 0, 1};
    grid.offset = {x, 0, z};
    grid.halfWidth = 1000;
    grid.spacing = spacing;
    grid.guidelineSpacing = guidelines;
    drawDebugGrid(grid);
}

void up::shell::SceneEditor::_resize(GpuDevice& device, glm::ivec2 size) {
    GpuTextureDesc desc;
    desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
    desc.type = GpuTextureType::Texture2D;
    desc.width = size.x;
    desc.height = size.y;
    _buffer = device.createTexture2D(desc, {});

    _bufferView = device.createShaderResourceView(_buffer.get());
}

void up::shell::SceneEditor::_inspector() {
    ComponentId deletedComponent = ComponentId::Unknown;

    if (_doc->scene() == nullptr) {
        return;
    }

    if (!ImGui::BeginTable(
            "##inspector_table",
            2,
            ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBodyUntilResize)) {
        return;
    }

    ImGuiID const addComponentId = ImGui::GetID("##add_component_list");

    _doc->scene()->world().interrogateEntityUnsafe(
        _selection.selected(),
        [&](EntityId entity, ArchetypeId archetype, reflex::TypeInfo const* typeInfo, auto* data) {
            ImGui::PushID(data);

            const bool open = _propertyGrid.beginItem(typeInfo->name.c_str());

            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                ImGui::OpenPopup("##component_context_menu");
            }

            if (ImGui::BeginPopupContextItem("##component_context_menu")) {
                if (ImGui::IconMenuItem("Add", ICON_FA_PLUS_CIRCLE)) {
                    ImGui::OpenPopupEx(addComponentId);
                }
                if (ImGui::IconMenuItem("Remove", ICON_FA_TRASH)) {
                    deletedComponent = static_cast<ComponentId>(typeInfo->hash);
                }
                ImGui::EndPopup();
            }

            if (open) {
                if (typeInfo->schema != nullptr) {
                    _propertyGrid.editObjectRaw(*typeInfo->schema, data);
                }
                _propertyGrid.endItem();
            }

            ImGui::PopID();
        });

    ImGui::EndTable();

    if (deletedComponent != ComponentId::Unknown) {
        _doc->scene()->world().removeComponent(_selection.selected(), deletedComponent);
    }

    if (_selection.hasSelection()) {
        if (ImGui::IconButton("Add Component", ICON_FA_PLUS_CIRCLE)) {
            ImGui::OpenPopupEx(addComponentId);
        }

        if (ImGui::BeginPopupEx(
                addComponentId,
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoMove)) {
            for (reflex::TypeInfo const* typeInfo : _components()) {
                if (_doc->scene()->world().getComponentSlowUnsafe(
                        _selection.selected(),
                        static_cast<ComponentId>(typeInfo->hash)) == nullptr) {
                    if (ImGui::IconMenuItem(typeInfo->name.c_str())) {
                        _doc->scene()->world().addComponentDefault(_selection.selected(), *typeInfo);
                    }
                }
            }
            ImGui::EndPopup();
        }
    }
}

void up::shell::SceneEditor::_hierarchy() {
    for (int index : _doc->indices()) {
        if (_doc->entityAt(index).parent == -1) {
            _hierarchyShowIndex(index);
        }
    }
}

void up::shell::SceneEditor::_hierarchyShowIndex(int index) {
    char label[128];

    SceneEntity const& ent = _doc->entityAt(index);

    format_to(label, "{} (#{})", !ent.name.empty() ? ent.name.c_str() : "Entity", ent.id);

    bool const hasChildren = ent.firstChild != -1;
    bool const selected = ent.id == _selection.selected();

    unsigned flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
    if (!hasChildren) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (selected) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    ImGui::PushID(index);
    bool const open = ImGui::TreeNodeEx(label, flags);
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        _selection.select(ent.id);
    }

    if (selected) {
        ImGui::SetItemDefaultFocus();
    }

    if (open) {
        for (int childIndex = ent.firstChild; childIndex != -1; childIndex = _doc->entityAt(childIndex).nextSibling) {
            _hierarchyShowIndex(childIndex);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}
