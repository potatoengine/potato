// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "scene.h"

#include "potato/ecs/common.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

namespace up {
    class Model;
    class SoundResource;

    struct SceneEntity {
        string name;
        EntityId id = EntityId::None;
        int firstChild = -1;
        int nextSibling = -1;
        int parent = -1;
    };

    class SceneDocument {
    public:
        explicit SceneDocument(rc<Scene> scene) : _scene(std::move(scene)) {}

        rc<Scene> const& scene() const { return _scene; }

        sequence<int> indices() const noexcept { return sequence{static_cast<int>(_entities.size())}; }
        SceneEntity const& entityAt(int index) const noexcept { return _entities[index]; }
        int indexOf(EntityId entityId) const noexcept;

        EntityId createEntity(string name, EntityId parentId = EntityId::None);
        void deleteEntity(EntityId targetId);

        void parentTo(EntityId childId, EntityId parentId);

        void createTestObjects(rc<Model> const& cube, rc<SoundResource> const& ding);

    private:
        rc<Scene> _scene;
        vector<SceneEntity> _entities;
    };
} // namespace up