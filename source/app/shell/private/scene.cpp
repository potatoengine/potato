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

    struct Animation {
        float offset;
        float speedScale;
        float time;
    };
} // namespace up::components

UP_COMPONENT(up::components::Position);
UP_COMPONENT(up::components::Transform);
UP_COMPONENT(up::components::Mesh);
UP_COMPONENT(up::components::Animation);

up::Scene::Scene() : _world(new_box<World>()) {
}

void up::Scene::create(rc<Model> cube) {
    auto pi = glm::pi<float>();

    for (size_t i = 0; i <= 100; ++i) {
        float p = i / 100.0f;
        float r = p * 2.f * pi;
        _world->createEntity(
            components::Position{{
                (20 + glm::cos(r) * 10.f) * glm::sin(r),
                1 + glm::sin(r * 10) * 5,
                (20 + glm::sin(r) * 10.f) * glm::cos(r)}
            },
            components::Transform{},
            components::Mesh{cube},
            components::Animation{glm::sin(r * 10.f), .2f + glm::sin(r), 0}
        );
    }

    _world->createEntity(
        components::Position{{0, 5, 0}},
        components::Transform(),
        components::Mesh{cube}
    );
}

up::Scene::~Scene() {
    _cube.reset();
}

void up::Scene::tick(float frameTime) {
    Query<components::Position, components::Animation> tickQuery;
    Query<components::Position, components::Transform> transformQuery;

    tickQuery.select(*_world, [&](size_t count, EntityId const*, components::Position* positions, components::Animation* animations) {
        for (size_t i = 0; i != count; ++i) {
            animations[i].time += frameTime * animations[i].speedScale;
            positions[i].xyz = glm::rotateY(positions[i].xyz, frameTime);
            positions[i].xyz.y = 1 + 10 * glm::sin(animations[i].offset + 2.f * glm::pi<float>() * animations[i].time);
        }
    });

    transformQuery.select(*_world, [&](size_t count, EntityId const*, components::Position* positions, components::Transform* transforms) {
        for (size_t i = 0; i != count; ++i) {
            transforms[i].trans = glm::translate(glm::identity<glm::mat4x4>(), positions[i].xyz);
        }
    });
}

void up::Scene::render(RenderContext& ctx) {
    Query<components::Mesh, components::Transform> renderableMeshQuery;

    renderableMeshQuery.select(*_world, [&](size_t count, EntityId const*, components::Mesh* meshes, components::Transform* transforms) {
        for (size_t i = 0; i != count; ++i) {
                meshes[i].model->render(ctx, transforms[i].trans);
            }
    });
}
