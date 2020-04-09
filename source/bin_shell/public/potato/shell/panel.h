// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

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

        virtual void render(Renderer& renderer, float frameTime) = 0;
        virtual void ui() = 0;
        virtual void tick(float deltaTime) = 0;
        virtual auto handleEvent(SDL_Event const& ev) -> bool = 0;
    };
}
