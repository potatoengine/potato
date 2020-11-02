// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/ecs/query.h"
#include "potato/ecs/universe.h"
#include "potato/runtime/stream.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"

namespace up::schema {
    struct Transform;
    struct Mesh;
    struct Wave;
    struct Spin;
    struct Ding;
} // namespace up::schema

namespace up {
    class Model;
    class Node;
    class RenderContext;
    class AudioEngine;
    class SoundResource;

    class Scene : public shared<Scene> {
    public:
        explicit Scene(Universe& universe, AudioEngine& audioEngine);
        ~Scene();

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        void create(rc<Model> const& cube, rc<SoundResource> const& ding);
        void tick(float frameTime);
        void flush();
        void render(RenderContext& ctx);

        bool load(Stream file);
        void save(Stream file);

        bool playing() const { return _playing; }
        bool playing(bool active) { return _playing = active; }

        World& world() noexcept { return _world; }
        EntityId root() const noexcept { return _root; }

    private:
        AudioEngine& _audioEngine;
        rc<Model> _cube;
        World _world;
        EntityId _root = EntityId::None;
        bool _playing = false;

        Query<schema::Transform, schema::Wave> _waveQuery;
        Query<schema::Transform> _orbitQuery;
        Query<schema::Transform, schema::Spin> _spinQuery;
        Query<schema::Ding> _dingQuery;
        Query<schema::Transform> _transformQuery;
        Query<schema::Mesh, schema::Transform> _renderableMeshQuery;
    };
} // namespace up
