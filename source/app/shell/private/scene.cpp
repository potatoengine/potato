// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "scene.h"
#include <potato/render/model.h>
#include <potato/render/node.h>
#include "potato/ecs/world.h"
#include "potato/ecs/query.h"

#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace up::components {
    struct Position {
        glm::vec3 xyz;
    };

    struct Transform {
        glm::mat4x4 trans;
    };

    struct Mesh {
        rc<up::Model> model;
    };
} // namespace up::components

UP_COMPONENT(up::components::Position);
UP_COMPONENT(up::components::Transform);
UP_COMPONENT(up::components::Mesh);

up::Scene::Scene() : _world(new_box<World>()) {
}

void up::Scene::create(rc<Model> cube) {
    for (size_t i = 0; i != 100; ++i) {
        float p = i / 100.0f;
        _world->createEntity(
            components::Position{{(20 + glm::cos(p * 20.f) * 10.f) * glm::sin(p * 2.f * glm::pi<float>()), 1 + glm::sin(p * 10) * 5, (20 + glm::sin(p * 20.f) * 10.f) * glm::cos(p * 2.f * glm::pi<float>())}},
            components::Transform{},
            components::Mesh{cube});
    }
}

up::Scene::~Scene() {
    _cube.reset();
}

void up::Scene::render(RenderContext& ctx) {
    Query<components::Mesh, components::Transform> renderableMeshQuery;

    renderableMeshQuery.select(*_world, [&](size_t count, EntityId const*, components::Mesh* meshes, components::Transform* transforms) {
        for (size_t i = 0; i != count; ++i) {
            meshes[i].model->render(ctx, transforms[i].trans);
        }
    });
}

void up::Scene::tick(float frameTime) {
    Query<components::Position, components::Transform> transformUpdateQuery;
    Query<components::Position> rotationQuery;

    rotationQuery.select(*_world, [frameTime](size_t count, EntityId const*, components::Position* positions) {
        for (size_t i = 0; i != count; ++i) {
            positions[i].xyz = glm::rotateY(positions[i].xyz, frameTime);
        }
    });

    transformUpdateQuery.select(*_world, [](size_t count, EntityId const*, components::Position* positions, components::Transform* transforms) {
        for (size_t i = 0; i != count; ++i) {
            transforms[i].trans = glm::translate(glm::identity<glm::mat4x4>(), positions[i].xyz);
        }
    });
}
