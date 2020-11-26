// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.
//
// This file includes code from dear imgui at https://github.com/ocornut/imgui, which is
// covered by the following license.
//
// The MIT License (MIT)
//
// Copyright(c) 2014 - 2020 Omar Cornut
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "imgui_ext.h"
#include "imgui_backend.h"

#include "potato/spud/string_format.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>
#include <imgui.h>
#include <imgui_internal.h>

static void DrawIcon(const char8_t* icon, ImVec2 minPos, ImVec2 maxPos) {
    ImGui::RenderTextClipped(
        minPos,
        maxPos,
        reinterpret_cast<char const*>(icon),
        nullptr,
        nullptr,
        {0.5f, 0.5f},
        nullptr);
}

bool ImGui::Potato::IconButton(char const* label, char8_t const* icon, ImVec2 size, ImGuiButtonFlags flags) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiContext& g = *GImGui;
    ImGuiStyle const& style = g.Style;
    ImFont const* font = ImGui::GetFont();

    float const iconWidth = font->FontSize;
    ImVec2 const iconSize{iconWidth, iconWidth};

    bool const hasLabel = FindRenderedTextEnd(label) != label;
    ImVec2 const labelSize = hasLabel ? CalcTextSize(label, nullptr, true) : ImVec2{};

    ImVec2 pos = window->DC.CursorPos;

    if (window != nullptr && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) {
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    }
    ImVec2 buttonSize = CalcItemSize(
        size,
        labelSize.x + iconWidth + style.FramePadding.x * 2.0f + (hasLabel ? style.ItemInnerSpacing.x : 0.f),
        std::max(labelSize.y, iconSize.y) + style.FramePadding.y * 2.0f);

    ImRect const bb(pos, pos + buttonSize);
    ItemSize(buttonSize, style.FramePadding.y);

    ImGuiID const id = window->GetID(label);
    if (!ItemAdd(bb, id)) {
        return false;
    }

    if ((window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat) != 0) {
        flags |= ImGuiButtonFlags_Repeat;
    }

    bool hovered = false;
    bool held = false;
    bool const pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    ImU32 const col =
        GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
    if (icon != nullptr && *icon != u8'\0') {
        DrawIcon(icon, bb.Min + style.FramePadding, bb.Min + style.FramePadding + iconSize);
    }
    RenderTextClipped(
        bb.Min + ImVec2{iconWidth + style.ItemInnerSpacing.x, 0} + style.FramePadding,
        bb.Max - style.FramePadding,
        label,
        nullptr,
        &labelSize,
        style.ButtonTextAlign,
        &bb);

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

bool ImGui::Potato::IsModifierDown(ImGuiKeyModFlags modifiers) noexcept {
    auto& io = ImGui::GetIO();
    return (io.KeyMods & modifiers) == modifiers;
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

bool ImGui::BeginIconMenu(const char* label, bool enabled) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal) {
        return BeginMenu(label, enabled);
    }

    char const* labelEnd = FindRenderedTextEnd(label);

    ImVec2 const pos = window->DC.CursorPos;
    ImVec2 const iconSize = ImGui::CalcTextSize(reinterpret_cast<char const*>(ICON_FA_INFO));
    ImVec2 const labelSize = ImGui::CalcTextSize(label, labelEnd);
    ImVec2 const spacing = GetStyle().ItemInnerSpacing;

    float const labelOffsetX = iconSize.x + spacing.x * 2.f;

    ImColor const color = GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    window->DC.MenuColumns.DeclColumns(labelSize.x + labelOffsetX, 0, 0);

    char menuId[32] = {
        0,
    };
    up::format_to(menuId, "##menu_{}", GetID(label));

    bool const open = BeginMenu(menuId, enabled);

    // we need to draw into our original window, so temporarily close the new sub-menu,
    // draw, and then we'll reopen it if appropriate.
    //
    ImGuiWindow* menuWindow = nullptr;
    if (open) {
        menuWindow = GetCurrentWindow();
        EndPopup();
        UP_ASSERT(window == GetCurrentWindow());
    }

    window->DrawList->AddText(ImVec2(pos.x + labelOffsetX, pos.y), color, label, labelEnd);

    if (open) {
        BeginPopup(menuId, menuWindow->Flags);
    }

    return open;
}

void ImGui::Potato::IconMenuSeparator() {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return;
    }

    if (window->DC.LayoutType == ImGuiLayoutType_Vertical) {
        ImVec2 const iconSize = ImGui::CalcTextSize(reinterpret_cast<char const*>(ICON_FA_INFO));
        ImVec2 const spacing = GetStyle().ItemInnerSpacing;

        if (!window->DC.GroupStack.empty()) {
            window->DC.CursorPos.x += window->DC.Indent.x;
        }

        window->DC.CursorPos.x += iconSize.x + spacing.x * 2.f;

        ImRect const bounds(window->DC.CursorPos, ImVec2(window->Size.x - spacing.x, window->DC.CursorPos.y + 1.f));

        ItemSize(ImVec2(0.0f, 1.f));
        if (!ItemAdd(bounds, 0)) {
            return;
        }

        window->DrawList->AddLine(bounds.Min, ImVec2(bounds.Max.x, bounds.Min.y), GetColorU32(ImGuiCol_Separator));
    }
    else {
        Separator();
    }
}

bool ImGui::Potato::IconMenuItem(
    const char* label,
    const char8_t* icon,
    const char* shortcut,
    bool selected,
    bool enabled) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems && TableGetColumnCount() == 0) {
        return false;
    }

    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal) {
        return MenuItem(label, shortcut, selected, enabled);
    }

    char const* labelEnd = FindRenderedTextEnd(label);

    ImVec2 const pos = window->DC.CursorPos;
    ImVec2 const iconSize = ImGui::CalcTextSize(reinterpret_cast<char const*>(ICON_FA_INFO));
    ImVec2 const labelSize = ImGui::CalcTextSize(label, labelEnd);
    ImVec2 const spacing = GetStyle().ItemInnerSpacing;

    float const labelOffsetX = iconSize.x + spacing.x * 2.f;

    ImColor const color = GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    window->DC.MenuColumns.DeclColumns(labelSize.x + labelOffsetX, 0, 0);

    PushID(label);
    bool const clicked = MenuItem("##item", shortcut, &selected, enabled);
    PopID();

    if (icon != nullptr) {
        window->DrawList->AddText(ImVec2(pos.x + spacing.x, pos.y), color, reinterpret_cast<char const*>(icon));
    }

    window->DrawList->AddText(ImVec2(pos.x + labelOffsetX, pos.y), color, label, labelEnd);

    return clicked;
}

void ImGui::Potato::TextCentered(ImVec2 minPos, ImVec2 maxPos, ImU32 color, char const* text, char const* end) {
    ImGuiWindow const* const window = GetCurrentWindow();
    ImFont const* const font = GetFont();

    ImVec4 const bounds{minPos.x, minPos.y, maxPos.x, maxPos.y};
    ImVec2 const size = CalcTextSize(text, end, false, 0.f);
    float const width = maxPos.x - minPos.x;

    if (size.x >= width) {
        window->DrawList->AddText(font, font->FontSize, minPos, color, text, end, 0.f, &bounds);
    }
    else {
        float const offsetX = (width - size.x) * 0.5f;
        ImVec2 const pos{minPos.x + offsetX, minPos.y};
        window->DrawList->AddText(font, font->FontSize, pos, color, text, end, 0.f, &bounds);
    }
}

ImVec2 ImGui::Potato::GetItemSpacing() {
    return GetStyle().ItemSpacing;
}

ImVec2 ImGui::Potato::GetItemInnerSpacing() {
    return GetStyle().ItemInnerSpacing;
}
