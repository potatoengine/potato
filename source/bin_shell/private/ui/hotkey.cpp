// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "commands.h"
#include "ui/hotkey.h"

#include <SDL_keycode.h>
#include <imgui.h>

void up::shell::HotKeys::addHotKey(HotKeyDesc desc) {
    _hotKeys.push_back(std::move(desc));
}

bool up::shell::HotKeys::evaluateKey(CommandRegistry& commands, int keysym, unsigned mods) {
    if (keysym == 0) {
        return false;
    }

    // Normalized mods so left-v-right is erased
    //
    mods &= KMOD_SHIFT | KMOD_CTRL | KMOD_ALT | KMOD_GUI;

    if (0 != (mods & KMOD_SHIFT)) {
        mods |= KMOD_SHIFT;
    }
    if (0 != (mods & KMOD_CTRL)) {
        mods |= KMOD_CTRL;
    }
    if (0 != (mods & KMOD_ALT)) {
        mods |= KMOD_ALT;
    }
    if (0 != (mods & KMOD_GUI)) {
        mods |= KMOD_GUI;
    }

    // Attempt to match the input with registered hot keys
    //
    for (auto const& desc : _hotKeys) {
        if (desc.key == keysym && desc.mods == mods) {
            auto const rs = commands.execute(desc.command);
            return rs == CommandResult::Okay;
        }
    }

    return false;
}
