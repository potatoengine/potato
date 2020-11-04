// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "ui/property_grid.h"
#include "common_schema.h"
#include "tools_schema.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <imgui.h>

void up::shell::PropertyGrid::drawGridRaw(zstring_view name, reflex::Schema const& schema, void* object) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

    drawObjectEditor(schema, object);

    ImGui::PopStyleVar();
}

void up::shell::PropertyGrid::drawEditor(reflex::Schema const& schema, void* object) {
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

void up::shell::PropertyGrid::drawObjectEditor(reflex::Schema const& schema, void* object) {
    UP_ASSERT(schema.primitive == reflex::SchemaPrimitive::Object);
    for (reflex::SchemaField const& field : schema.fields) {
        drawPropertyRaw(field, object);
    }
}

void up::shell::PropertyGrid::drawPropertyRaw(reflex::SchemaField const& field, void* object) {
    void* const member = static_cast<char*>(object) + field.offset;
    auto const* const displayNameAnnotation = field.queryAnnotation<schema::DisplayName>();
    zstring_view const displayName = displayNameAnnotation != nullptr && !displayNameAnnotation->name.empty()
        ? displayNameAnnotation->name
        : field.name;

    ImGui::PushID(object);
    ImGui::AlignTextToFramePadding();
    bool const open = ImGui::TreeNode(displayName.c_str());

    ImGui::NextColumn();
    if (open) {
        drawEditor(*field.schema, object);
        ImGui::TreePop();
    }
    ImGui::NextColumn();

    ImGui::PopID();
}

void up::shell::PropertyGrid::drawIntEditor(int& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputInt("##int", &value);
}

void up::shell::PropertyGrid::drawFloatEditor(float& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat("##float", &value);
}

void up::shell::PropertyGrid::drawVec3Editor(glm::vec3& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat3("##vec3", &value.x);
}

void up::shell::PropertyGrid::drawMat4x4Editor(glm::mat4x4& value) noexcept {
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat4("##a", &value[0].x);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat4("##b", &value[1].x);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat4("##c", &value[2].x);
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputFloat4("##d", &value[3].x);
}

void up::shell::PropertyGrid::drawQuatEditor(glm::quat& value) noexcept {
    auto euler = glm::eulerAngles(value);
    auto eulerDegrees = glm::vec3(glm::degrees(euler.x), glm::degrees(euler.y), glm::degrees(euler.z));

    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::SliderFloat3("##quat", &eulerDegrees.x, 0, +359.f)) {
        value = glm::vec3(glm::radians(eulerDegrees.x), glm::radians(eulerDegrees.y), glm::radians(eulerDegrees.z));
    }
}
