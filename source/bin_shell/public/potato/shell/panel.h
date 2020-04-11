// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include <potato/spud/zstring_view.h>

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

        virtual zstring_view displayName() const = 0;
        virtual void ui() = 0;

        bool enabled() const noexcept { return _enabled; }
        void enabled(bool enabled) noexcept { _enabled = enabled; }

    private:
        bool _enabled = true;
    };
}
