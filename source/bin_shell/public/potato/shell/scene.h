// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/ecs/query.h"
#include "potato/ecs/universe.h"
#include "potato/runtime/stream.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/render/renderer.h"
#include <glm/gtc/matrix_transform.hpp>

namespace up::components {
    struct Transform;
    struct Mesh;
    struct Wave;
    struct Spin;
    struct Ding;
} // namespace up::components

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

        class MeshRenderer : public up::IRenderable {
        public:
            MeshRenderer(Query<components::Mesh, components::Transform>* meshQuery, World* world)
                :_meshQuery(meshQuery)
                ,_world(world)
                {}

            void onSchedule(up::RenderContext& ctx) override;
            void onRender(up::RenderContext& ctx) override;

        private:
            Query<components::Mesh, components::Transform>* _meshQuery;
            World* _world;
        };


        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        void create(rc<Model> const& cube, rc<SoundResource> const& ding);
        void tick(float frameTime);
        void flush();
        void render(Renderer& renderer);

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

        box<MeshRenderer> _meshRenderer;

        Query<components::Transform, components::Wave> _waveQuery;
        Query<components::Transform> _orbitQuery;
        Query<components::Transform, components::Spin> _spinQuery;
        Query<components::Ding> _dingQuery;
        Query<components::Transform> _transformQuery;
        Query<components::Mesh, components::Transform> _renderableMeshQuery;
    };
} // namespace up
