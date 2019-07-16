// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include <potato/foundation/box.h>
#include <potato/foundation/rc.h>

namespace up {
    class Model;
    class Node;
    class RenderContext;
    class World;

    class Scene {
    public:
        Scene();
        ~Scene();

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        void create(rc<Model> cube);
        void render(RenderContext& ctx);
        void tick(float frameTime);

    private:
        rc<Model> _cube;
        box<World> _world;
    };
} // namespace up
