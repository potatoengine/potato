// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include <potato/spud/box.h>
#include <potato/spud/rc.h>
#include <potato/filesystem/stream.h>
#include <potato/ecs/query.h>

namespace up::components {
    struct Position;
    struct Rotation;
    struct Transform;
    struct Mesh;
    struct Wave;
    struct Spin;
} // namespace up::components

namespace up {
    class Model;
    class Node;
    class RenderContext;

    class Scene {
    public:
        Scene();
        ~Scene();

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        void create(rc<Model> cube);
        void tick(float frameTime);
        void flush();
        void render(RenderContext& ctx);

        bool load(Stream file);
        void save(Stream file);

    private:
        rc<Model> _cube;
        box<World> _world;

        Query<components::Position, components::Wave> _waveQuery;
        Query<components::Position> _orbitQuery;
        Query<components::Rotation, components::Spin> _spinQuery;
        Query<components::Rotation, components::Position, components::Transform> _transformQuery;
        Query<components::Mesh, components::Transform> _renderableMeshQuery;
    };
} // namespace up
