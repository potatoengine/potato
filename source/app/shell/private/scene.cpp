// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "scene.h"
#include <potato/render/model.h>
#include <potato/render/node.h>
#include "potato/ecs/world.h"
#include "potato/ecs/query.h"

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace up::components {
    struct Position {
        glm::vec3 xyz;
    };

    struct Rotation {
        glm::quat rot;
    };

    struct Transform {
        glm::mat4x4 trans;
    };

    struct Mesh {
        rc<up::Model> model;
    };

    struct Wave {
        float time;
        float offset;
    };

    struct Spin {
        float radians;
    };
} // namespace up::components

UP_COMPONENT(up::components::Position);
UP_COMPONENT(up::components::Rotation);
UP_COMPONENT(up::components::Transform);
UP_COMPONENT(up::components::Mesh);
UP_COMPONENT(up::components::Wave);
UP_COMPONENT(up::components::Spin);

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
                1 + glm::sin(r * 10.f) * 5.f,
                (20 + glm::sin(r) * 10.f) * glm::cos(r)}
            },
            components::Rotation{glm::identity<glm::quat>()},
            components::Transform{},
            components::Mesh{cube},
            components::Wave{0, r},
            components::Spin{glm::sin(r * 10.f) * 2.f - 1.f}
        );
    }

    _world->createEntity(
        components::Position{{0, 5, 0}},
        components::Rotation{glm::identity<glm::quat>()},
        components::Transform(),
        components::Mesh{cube}
    );
}

up::Scene::~Scene() {
    _cube.reset();
}

void up::Scene::tick(float frameTime) {
    _waveQuery.select(*_world, [&](size_t count, EntityId const*, components::Position* positions, components::Wave* waves) {
        for (size_t i = 0; i != count; ++i) {
            waves[i].offset += frameTime * .2f;
            positions[i].xyz.y = 1 + 5 * glm::sin(waves[i].offset * 10);
        }
    });

    _orbitQuery.select(*_world, [&](size_t count, EntityId const*, components::Position* positions) {
        for (size_t i = 0; i != count; ++i) {
            positions[i].xyz = glm::rotateY(positions[i].xyz, frameTime);
        }
    });

    _spinQuery.select(*_world, [&](size_t count, EntityId const*, components::Rotation* rotations, components::Spin* spins) {
        for (size_t i = 0; i != count; ++i) {
            rotations[i].rot = glm::angleAxis(spins[i].radians * frameTime, glm::vec3(0.f, 1.f, 0.f)) * rotations[i].rot;
        }
    });
}

void up::Scene::flush() {
    _transformQuery.select(*_world, [&](size_t count, EntityId const*, components::Rotation* rotations, components::Position* positions, components::Transform* transforms) {
        for (size_t i = 0; i != count; ++i) {
            transforms[i].trans = glm::translate(positions[i].xyz) * glm::mat4_cast(rotations[i].rot);
        }
    });
}

void up::Scene::render(RenderContext& ctx) {
    _renderableMeshQuery.select(*_world, [&](size_t count, EntityId const*, components::Mesh* meshes, components::Transform* transforms) {
        for (size_t i = 0; i != count; ++i) {
                meshes[i].model->render(ctx, transforms[i].trans);
            }
    });
}
