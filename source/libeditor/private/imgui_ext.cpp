// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "imgui_ext.h"
#include "imgui_backend.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>
#include <imgui.h>
#include <imgui_internal.h>

bool ImGui::Potato::MenuItemEx(
    const char* label,
    const char8_t* icon,
    const char* shortcut,
    bool selected,
    bool enabled) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiContext& g = *GImGui;
    ImGuiStyle const& style = g.Style;
    ImFont const* font = ImGui::GetFont();

    float const iconWidth = font->FontSize;
    ImVec2 const labelSize = CalcTextSize(label, nullptr, true);

    ImGuiSelectableFlags const flags = ImGuiSelectableFlags_SelectOnRelease | ImGuiSelectableFlags_SetNavIdOnHover |
        (enabled ? 0 : ImGuiSelectableFlags_Disabled);

    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal) {
        float const width = labelSize.x + iconWidth;
        float const offset = IM_FLOOR(style.ItemSpacing.x * 0.5f);

        window->DC.CursorPos.x += offset;
        ImVec2 const iconPos = window->DC.CursorPos;

        PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x * 2.0f, style.ItemSpacing.y));
        bool const pressed = Selectable(label, false, flags, ImVec2(width, 0.0f));
        PopStyleVar();

        if (icon != nullptr && *icon != u8'\0') {
            window->DrawList->AddText(
                font,
                font->FontSize,
                iconPos,
                ImGui::GetColorU32(ImGuiCol_Header),
                reinterpret_cast<char const*>(icon));
        }

        window->DC.CursorPos.x += IM_FLOOR(
            style.ItemSpacing.x *
            (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would
                             // also work to call SameLine() ourselves after the PopStyleVar().

        return pressed;
    }

    ImVec2 const pos = window->DC.CursorPos;
    float const shortcutWidth = shortcut != nullptr ? CalcTextSize(shortcut, nullptr).x : 0.0f;
    float const minWidth = window->DC.MenuColumns.DeclColumns(
        labelSize.x + iconWidth,
        shortcutWidth,
        IM_FLOOR(g.FontSize * 1.20f)); // Feedback for next frame
    float const extraWidth = ImMax(0.0f, GetContentRegionAvail().x - minWidth);

    bool const pressed = Selectable(label, false, flags | ImGuiSelectableFlags_SpanAvailWidth, ImVec2(minWidth, 0.0f));
    if (icon != nullptr && *icon != u8'\0') {
        window->DrawList->AddText(
            font,
            font->FontSize,
            pos,
            ImGui::GetColorU32(ImGuiCol_Header),
            reinterpret_cast<char const*>(icon));
    }

    if (shortcutWidth > 0.0f) {
        PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
        RenderText(pos + ImVec2(window->DC.MenuColumns.Pos[1] + extraWidth, 0.0f), shortcut, nullptr, false);
        PopStyleColor();
    }

    if (selected) {
        RenderCheckMark(
            window->DrawList,
            pos + ImVec2(window->DC.MenuColumns.Pos[2] + extraWidth + g.FontSize * 0.40f, g.FontSize * 0.134f * 0.5f),
            GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled),
            g.FontSize * 0.866f);
    }

    return pressed;
}

void ImGui::Potato::SetCaptureRelativeMouseMode(bool captured) {
    auto& io = ImGui::GetIO();
    auto* const state = static_cast<up::ImguiBackend*>(io.UserData);
    if (state != nullptr) {
        state->setCaptureRelativeMouseMode(captured);
    }
}

auto ImGui::Potato::IsCaptureRelativeMouseMode() -> bool {
    auto& io = ImGui::GetIO();
    auto* const state = static_cast<up::ImguiBackend*>(io.UserData);
    return state != nullptr ? state->isCaptureRelativeMouseMode() : false;
}

bool ImGui::Potato::InputVec3(char const* label, glm::vec3& value, char const* format, ImGuiInputTextFlags flags) {
    return ImGui::InputFloat3(label, &value.x, format, flags);
}

bool ImGui::Potato::InputQuat(char const* label, glm::quat& value, char const* format, ImGuiSliderFlags flags) {
    auto euler = glm::eulerAngles(value);
    auto eulerDegrees = glm::vec3(glm::degrees(euler.x), glm::degrees(euler.y), glm::degrees(euler.z));

    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::SliderFloat3(label, &eulerDegrees.x, 0.f, +359.f, format, flags)) {
        value = glm::vec3(glm::radians(eulerDegrees.x), glm::radians(eulerDegrees.y), glm::radians(eulerDegrees.z));
        return true;
    }

    return false;
}
