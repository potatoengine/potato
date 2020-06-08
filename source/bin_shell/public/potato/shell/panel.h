// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/spud/zstring_view.h"

using SDL_Event = union SDL_Event;

namespace up {
    class Renderer;
}

namespace up::shell {
    class Panel {
    public:
        Panel() = default;
        virtual ~Panel() = default;

        Panel(Panel const&) = delete;
        Panel& operator=(Panel const&) = delete;

        /// @brief Display name of the panel, used in menus and window titles.
        /// @return display name.
        virtual zstring_view displayName() const = 0;

        /// @brief Renders the ui for the panel. Only called when the panel is enabled.
        virtual void ui() = 0;

        /// @return true if the panel is enabled (visible).
        bool enabled() const noexcept { return _enabled; }

        /// @param enabled Whether the panel should be enabled or disabled.
        void enabled(bool enabled) noexcept { _enabled = enabled; }

    private:
        bool _enabled = true;
    };
} // namespace up::shell
