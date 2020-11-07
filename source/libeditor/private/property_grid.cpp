// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "property_grid.h"
#include "common_schema.h"
#include "tools_schema.h"

#include "potato/editor/imgui_ext.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <imgui.h>

void up::editor::PropertyGrid::drawGridRaw(zstring_view name, reflex::Schema const& schema, void* object) {
    drawObjectEditor(schema, object);
}

void up::editor::PropertyGrid::drawEditor(reflex::Schema const& schema, void* object) {
    ImGui::PushID(object);

    switch (schema.primitive) {
        case reflex::SchemaPrimitive::Int16:
            drawIntEditor(*static_cast<int16*>(object));
            break;
        case reflex::SchemaPrimitive::Float:
            drawFloatEditor(*static_cast<float*>(object));
            break;
        case reflex::SchemaPrimitive::Vec3:
            drawVec3Editor(*static_cast<glm::vec3*>(object));
            break;
        case reflex::SchemaPrimitive::Mat4x4:
            drawMat4x4Editor(*static_cast<glm::mat4x4*>(object));
            break;
        case reflex::SchemaPrimitive::Quat:
            drawQuatEditor(*static_cast<glm::quat*>(object));
            break;
        case reflex::SchemaPrimitive::Pointer:
            if (void* pointee = *static_cast<void**>(object); pointee != nullptr) {
                drawEditor(*schema.elementType, pointee);
            }
            break;
        case reflex::SchemaPrimitive::Object:
            drawObjectEditor(schema, object);
            break;
        default:
            ImGui::Text("Unsupported primitive type for schema `%s`", schema.name.c_str());
            break;
    }

    ImGui::PopID();
}

void up::editor::PropertyGrid::drawObjectEditor(reflex::Schema const& schema, void* object) {
    UP_ASSERT(schema.primitive == reflex::SchemaPrimitive::Object);
    for (reflex::SchemaField const& field : schema.fields) {
        drawPropertyRaw(field, object);
    }
}

void up::editor::PropertyGrid::drawPropertyRaw(reflex::SchemaField const& field, void* object) {
    if (field.queryAnnotation<schema::Hidden>() != nullptr) {
        return;
    }

    void* const member = static_cast<char*>(object) + field.offset;

    if (field.queryAnnotation<schema::Flatten>() != nullptr) {
        ImGui::PushID(member);
        drawEditor(*field.schema, member);
        ImGui::PopID();
        return;
    }

    auto const* const displayNameAnnotation = field.queryAnnotation<schema::DisplayName>();
    zstring_view const displayName = displayNameAnnotation != nullptr && !displayNameAnnotation->name.empty()
        ? displayNameAnnotation->name
        : field.name;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    bool const expandable =
        (field.schema->primitive == reflex::SchemaPrimitive::Object ||
         field.schema->primitive == reflex::SchemaPrimitive::Mat4x4);
    if (!expandable) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    ImGui::PushID(member);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    bool const open = ImGui::TreeNodeEx(displayName.c_str(), flags);

    ImGui::TableSetColumnIndex(1);
    ImGui::AlignTextToFramePadding();
    if (open) {
        drawEditor(*field.schema, member);
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void up::editor::PropertyGrid::drawIntEditor(int& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputInt("##int", &value);
}

void up::editor::PropertyGrid::drawFloatEditor(float& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat("##float", &value);
}

void up::editor::PropertyGrid::drawVec3Editor(glm::vec3& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputVec3("##vec3", value);
}

void up::editor::PropertyGrid::drawMat4x4Editor(glm::mat4x4& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat4("##a", &value[0].x);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat4("##b", &value[1].x);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat4("##c", &value[2].x);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat4("##d", &value[3].x);
}

void up::editor::PropertyGrid::drawQuatEditor(glm::quat& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputQuat("##quat", value);
}
