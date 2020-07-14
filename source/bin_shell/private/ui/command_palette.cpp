// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "command_palette.h"
#include "commands.h"

#include "potato/spud/sequence.h"

#include <SDL_keycode.h>
#include <imgui.h>
#include <imgui_internal.h>

void up::shell::CommandPalette::show() {
    _open = true;
}

void up::shell::CommandPalette::close() {
    _input[0] = '\0';
    _activeIndex = -1;
    _matches.clear();
    _open = false;
}

void up::shell::CommandPalette::update(CommandRegistry& registry) {
    auto& imguiIO = ImGui::GetIO();

    auto const popupName = "##CommandInput";
    auto const popupFlags = ImGuiWindowFlags_NoMove;

    // Handle opening of dialog
    //
    if (ImGui::IsKeyPressed(SDL_SCANCODE_P, false) && (imguiIO.KeyMods & ImGuiKeyModFlags_Ctrl) != 0) {
        show();
    }

    if (_open) {
        ImGui::OpenPopup(popupName);
    }

    // Measure width of the menu bar, used to offset the paletter
    //
    ImGui::BeginMainMenuBar();
    auto const menuSize = ImGui::GetWindowSize();
    ImGui::EndMainMenuBar();

    // Position and display the popup
    //
    auto const halfScreenWidth = imguiIO.DisplaySize.x * 0.5f;
    auto const dialogWidth = max(halfScreenWidth, 220.f);

    ImGui::SetNextWindowPos({halfScreenWidth - dialogWidth * 0.5f, menuSize.y});
    ImGui::SetNextWindowSize({dialogWidth, 0});

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    auto const isOpen = ImGui::BeginPopup(popupName, popupFlags);
    ImGui::PopStyleVar(1);

    // Render popup contents only if open
    //
    if (isOpen) {
        auto const inputName = "##command";
        auto const inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll |
            ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;

        // Display input
        //
        ImGui::SetNextItemWidth(dialogWidth);
        if (ImGui::IsWindowAppearing()) { // I cannot get SetItemDefaultFocus to work
            ImGui::SetKeyboardFocusHere();
        }

        auto const activated = ImGui::InputText(inputName, _input, sizeof(_input), inputFlags, &_callbackWrapper, this);

        // Update active index based on current input _before_ executing input
        //
        _updateMatches(registry);

        // Execute input if the input was activated (Enter pressed)
        //
        if (activated) {
            if (_input[0] == '\0' || _execute(registry)) {
                close();
            }

            // Ensure input remains active if Enter was pressed but no command was executed
            //
            if (_open) {
                ImGui::SetActiveID(ImGui::GetID(inputName), ImGui::GetCurrentContext()->CurrentWindow);
            }
        }

        // Unset our open state if the popup is closed
        //
        if (_open && !activated && ImGui::IsItemDeactivated()) {
            close();
        }

        // Draw matching commands
        //
        for (auto index : _matches) {
            bool const highlight = index == _activeIndex;
            ImGui::Selectable(registry.paletteDescs()[index].title.c_str(), highlight);
        }

        // Close popup if close is requested
        //
        if (!_open) {
            ImGui::CloseCurrentPopup();
        }
    }

    if (isOpen) {
        ImGui::EndPopup();
    }
}

auto up::shell::CommandPalette::_callbackWrapper(ImGuiInputTextCallbackData* data) -> int {
    return static_cast<CommandPalette*>(data->UserData)->_callback(data);
}

auto up::shell::CommandPalette::_callback(ImGuiInputTextCallbackData* data) -> int {
    switch (data->EventKey) {
        case ImGuiKey_Tab:
            return 0;
        case ImGuiKey_UpArrow:
            --_activeIndex;
            return 0;
        case ImGuiKey_DownArrow:
            ++_activeIndex;
            return 0;
        default:
            return 0;
    }
}

auto up::shell::CommandPalette::_execute(CommandRegistry& registry) const -> bool {
    if (_activeIndex == -1) {
        return false;
    }

    auto const* command = registry.commandAt(_activeIndex);
    if (command == nullptr) {
        return false;
    }

    auto const result = registry.execute(command->command);
    return result == CommandResult::Success;
}

void up::shell::CommandPalette::_updateMatches(CommandRegistry& registry) {
    _matches.clear();

    if (_input[0] == '\0') {
        _activeIndex = -1;
        return;
    }

    int lastMatchIndex = -1;

    auto const input = string_view{_input};
    auto const descs = registry.paletteDescs();
    for (auto index : sequence(descs.size())) {
        auto const& desc = descs[index];

        if (!registry.isExecutable(desc.command)) {
            continue;
        }

        if (stringIndexOfNoCase(desc.title.data(), desc.title.size(), input.data(), input.size()) != -1) {
            int const matchIndex = narrow_cast<int>(index);

            if (matchIndex == _activeIndex || lastMatchIndex == -1) {
                lastMatchIndex = matchIndex;
            }

            _matches.push_back(matchIndex);
        }
    }

    _activeIndex = lastMatchIndex;
}
