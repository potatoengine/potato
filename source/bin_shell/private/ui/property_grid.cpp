// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "ui/property_grid.h"
#include "common_schema.h"
#include "tools_schema.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <imgui.h>

void up::shell::PropertyGrid::drawGridRaw(zstring_view name, reflex::Schema const& schema, void* object) {
    switch (schema.primitive) {
        case reflex::SchemaPrimitive::Int16:
            return drawIntEditor(name, *static_cast<int16*>(object));
        case reflex::SchemaPrimitive::Float:
            return drawFloatEditor(name, *static_cast<float*>(object));
        case reflex::SchemaPrimitive::Vec3:
            return drawVec3Editor(name, *static_cast<glm::vec3*>(object));
        case reflex::SchemaPrimitive::Mat4x4:
            return drawMat4x4Editor(name, *static_cast<glm::mat4x4*>(object));
        case reflex::SchemaPrimitive::Quat:
            return drawQuatEditor(name, *static_cast<glm::quat*>(object));
        case reflex::SchemaPrimitive::Pointer:
            if (void* pointee = *static_cast<void**>(object); pointee != nullptr) {
                drawGridRaw(name, *schema.elementType, pointee);
            }
            return;
        case reflex::SchemaPrimitive::Object:
            if (name.empty() || ImGui::TreeNodeEx(name.c_str())) {
                for (reflex::SchemaField const& field : schema.fields) {
                    drawPropertyRaw(field, object);
                }
                if (!name.empty()) {
                    ImGui::TreePop();
                }
            }
            return;
        default:
            ImGui::Text("Unsupported primitive type for `%s` [schema: %s]", name.c_str(), schema.name.c_str());
            return;
    }
}

void up::shell::PropertyGrid::drawPropertyRaw(reflex::SchemaField const& field, void* object) {
    void* member = static_cast<char*>(object) + field.offset;
    auto const* displayName = field.schema->queryAnnotation<schema::DisplayName>();
    return drawGridRaw(displayName != nullptr ? displayName->name : field.name, *field.schema, object);
}

void up::shell::PropertyGrid::drawIntEditor(zstring_view name, int& value) noexcept {
    ImGui::InputInt(name.c_str(), &value);
}

void up::shell::PropertyGrid::drawFloatEditor(zstring_view name, float& value) noexcept {
    ImGui::InputFloat(name.c_str(), &value);
}

void up::shell::PropertyGrid::drawVec3Editor(zstring_view name, glm::vec3& value) noexcept {
    ImGui::InputFloat3(name.c_str(), &value.x);
}

void up::shell::PropertyGrid::drawMat4x4Editor(zstring_view name, glm::mat4x4& value) noexcept {
    if (ImGui::TreeNodeEx(name.c_str())) {
        ImGui::InputFloat4("##a", &value[0].x);
        ImGui::InputFloat4("##b", &value[1].x);
        ImGui::InputFloat4("##c", &value[2].x);
        ImGui::InputFloat4("##d", &value[3].x);
        ImGui::TreePop();
    }
}

void up::shell::PropertyGrid::drawQuatEditor(zstring_view name, glm::quat& value) noexcept {
    auto euler = glm::eulerAngles(value);
    auto eulerDegrees = glm::vec3(glm::degrees(euler.x), glm::degrees(euler.y), glm::degrees(euler.z));

    if (ImGui::SliderFloat3(name.c_str(), &eulerDegrees.x, 0, +359.f)) {
        value = glm::vec3(glm::radians(eulerDegrees.x), glm::radians(eulerDegrees.y), glm::radians(eulerDegrees.z));
    }
}
