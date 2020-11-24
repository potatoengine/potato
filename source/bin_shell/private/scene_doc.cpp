// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "scene_doc.h"
#include "components_schema.h"

#include "potato/ecs/world.h"
#include "potato/render/model.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>

int up::SceneDocument::indexOf(EntityId entityId) const noexcept {
    for (int index : indices()) {
        if (_entities[index].id == entityId) {
            return index;
        }
    }
    return -1;
}

up::EntityId up::SceneDocument::createEntity(string name, EntityId parentId) {
    EntityId const id = _scene->world().createEntity();
    _entities.push_back({.name = std::move(name), .id = id});
    if (parentId != EntityId::None) {
        parentTo(id, parentId);
    }
    return id;
}

void up::SceneDocument::deleteEntity(EntityId targetId) {
    auto const index = indexOf(targetId);
    if (index == -1) {
        return;
    }

    // recursively delete childen - we have to buffer this since deleting a child
    // would mutate the list we're walking
    vector<EntityId> deleted{targetId};
    _deleteEntityAt(index, deleted);

    for (EntityId const childId : deleted) {
        _entities.erase(_entities.begin() + indexOf(childId));
        _scene->world().deleteEntity(childId);
    }
}

void up::SceneDocument::_deleteEntityAt(int index, vector<EntityId>& out_deleted) {
    // reparent to ensure we're unlinked from a parent's chain
    parentTo(_entities[index].id, EntityId::None);

    while (_entities[index].firstChild != -1) {
        out_deleted.push_back(_entities[_entities[index].firstChild].id);
        _deleteEntityAt(_entities[index].firstChild, out_deleted);
    }
}

void up::SceneDocument::parentTo(EntityId childId, EntityId parentId) {
    int const childIndex = indexOf(childId);
    if (childIndex == -1) {
        return;
    }
    SceneEntity& childEnt = _entities[childIndex];

    int const parentIndex = indexOf(parentId);
    if (parentIndex == childEnt.parent) {
        return;
    }

    // remove from old parent
    if (childEnt.parent != -1) {
        SceneEntity& oldParent = _entities[childEnt.parent];
        if (oldParent.firstChild == childIndex) {
            oldParent.firstChild = childEnt.nextSibling;
        }
        else {
            for (int index = oldParent.firstChild; index != -1; index = _entities[index].nextSibling) {
                if (_entities[index].nextSibling == childIndex) {
                    _entities[index].nextSibling = childEnt.nextSibling;
                    break;
                }
            }
        }

        childEnt.parent = -1;
        childEnt.nextSibling = -1;
    }

    // add to new parent
    if (parentIndex != -1) {
        SceneEntity& newParent = _entities[parentIndex];
        if (newParent.firstChild == -1) {
            newParent.firstChild = childIndex;
        }
        else {
            for (int index = newParent.firstChild;; index = _entities[index].nextSibling) {
                if (_entities[index].nextSibling == -1) {
                    _entities[index].nextSibling = childIndex;
                    break;
                }
            }
        }

        childEnt.parent = parentIndex;
        childEnt.nextSibling = -1;
    }
}

void up::SceneDocument::createTestObjects(rc<Model> const& cube, rc<SoundResource> const& ding) {
    auto pi = glm::pi<float>();

    constexpr int numObjects = 100;

    auto const rootId = createEntity("Root");

    auto const centerId = createEntity("Center", rootId);
    _scene->world().addComponent(
        centerId,
        components::Transform{.position = {0, 5, 0}, .rotation = glm::identity<glm::quat>()});
    _scene->world().addComponent(centerId, components::Mesh{cube});
    _scene->world().addComponent(centerId, components::Ding{2, 0, ding});

    auto const ringId = createEntity("Ring", rootId);
    for (size_t i = 0; i <= numObjects; ++i) {
        float p = i / static_cast<float>(numObjects);
        float r = p * 2.f * pi;
        auto const id = createEntity("Orbit", ringId);
        _scene->world().addComponent(
            id,
            components::Transform{
                .position =
                    {(20 + glm::cos(r) * 10.f) * glm::sin(r),
                     1 + glm::sin(r * 10.f) * 5.f,
                     (20 + glm::sin(r) * 10.f) * glm::cos(r)},
                .rotation = glm::identity<glm::quat>()});
        _scene->world().addComponent(id, components::Mesh{cube});
        _scene->world().addComponent(id, components::Wave{0, r});
        _scene->world().addComponent(id, components::Spin{glm::sin(r) * 2.f - 1.f});
    }
}
