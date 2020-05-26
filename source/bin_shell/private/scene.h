// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/ecs/query.h"
#include "potato/ecs/universe.h"

#include "potato/runtime/stream.h"

#include "potato/spud/box.h"
#include "potato/spud/rc.h"

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
        explicit Scene(Universe& universe);
        ~Scene();

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        void create(rc<Model> const& cube);
        void tick(float frameTime);
        void flush();
        void render(RenderContext& ctx);

        bool load(Stream file);
        void save(Stream file);

        bool playing() const { return _playing; }
        bool playing(bool active) { return _playing = active; }

        World& world() noexcept { return _world; }
        EntityId main() const noexcept { return _main; }

    private:
        rc<Model> _cube;
        World _world;
        EntityId _main;
        bool _playing = false;

        Query<components::Position, components::Wave> _waveQuery;
        Query<components::Position> _orbitQuery;
        Query<components::Rotation, components::Spin> _spinQuery;
        Query<components::Rotation, components::Position, components::Transform> _transformQuery;
        Query<components::Mesh, components::Transform> _renderableMeshQuery;
    };
} // namespace up
