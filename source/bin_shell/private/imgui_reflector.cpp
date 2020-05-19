// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "imgui_reflector.h"

#include <imgui.h>

void up::shell::ImGuiComponentReflector::onField(up::zstring_view name) {
    _name = name;
}

void up::shell::ImGuiComponentReflector::onValue(int& value) {
    ImGui::InputInt(_name.c_str(), &value);
}

void up::shell::ImGuiComponentReflector::onValue(float& value) {
    ImGui::InputFloat(_name.c_str(), &value);
}

void up::shell::ImGuiComponentReflector::onValue(up::EntityId value) {
    ImGui::LabelText(_name.c_str(), "%u", (unsigned)value);
}

void up::shell::ImGuiComponentReflector::onValue(glm::vec3& value) {
    ImGui::InputFloat3(_name.c_str(), &value.x);
}

void up::shell::ImGuiComponentReflector::onValue(glm::quat& value) {
    auto euler = glm::eulerAngles(value);
    auto eulerDegrees = glm::vec3(
        glm::degrees(euler.x),
        glm::degrees(euler.y),
        glm::degrees(euler.z));

    if (ImGui::SliderFloat3(_name.c_str(), &eulerDegrees.x, 0, +359.f)) {
        value = glm::vec3(
            glm::radians(eulerDegrees.x),
            glm::radians(eulerDegrees.y),
            glm::radians(eulerDegrees.z));
    }
}

void up::shell::ImGuiComponentReflector::onValue(glm::mat4x4& value) {
    ImGui::InputFloat4("##a", &value[0].x);
    ImGui::InputFloat4("##b", &value[1].x);
    ImGui::InputFloat4("##c", &value[2].x);
    ImGui::InputFloat4("##d", &value[3].x);
}
