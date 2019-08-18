// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "scene.h"
#include <potato/filesystem/json.h>
#include <potato/render/model.h>
#include "potato/ecs/world.h"
#include "potato/ecs/query.h"
#include "potato/reflex/reflect.h"
#include "potato/reflex/json_serializer.h"

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <nlohmann/json.hpp>

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

UP_DECLARE_COMPONENT(up::components::Position);
UP_DECLARE_COMPONENT(up::components::Rotation);
UP_DECLARE_COMPONENT(up::components::Transform);
UP_DECLARE_COMPONENT(up::components::Mesh);
UP_DECLARE_COMPONENT(up::components::Wave);
UP_DECLARE_COMPONENT(up::components::Spin);

UP_DEFINE_COMPONENT(up::components::Position);
UP_DEFINE_COMPONENT(up::components::Rotation);
UP_DEFINE_COMPONENT(up::components::Transform);
UP_DEFINE_COMPONENT(up::components::Mesh);
UP_DEFINE_COMPONENT(up::components::Wave);
UP_DEFINE_COMPONENT(up::components::Spin);

UP_REFLECT_TYPE(up::components::Position) {
    reflect("xyz", &up::components::Position::xyz);
}

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
            components::Spin{glm::sin(r) * 2.f - 1.f}
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
    _waveQuery.select(*_world, [&](components::Position& pos, components::Wave& wave) {
        wave.offset += frameTime * .2f;
        pos.xyz.y = 1 + 5 * glm::sin(wave.offset * 10);
    });

    _orbitQuery.select(*_world, [&](components::Position& pos) {
        pos.xyz = glm::rotateY(pos.xyz, frameTime);
    });

    _spinQuery.select(*_world, [&](components::Rotation& rot, components::Spin const& spin) {
        rot.rot = glm::angleAxis(spin.radians * frameTime, glm::vec3(0.f, 1.f, 0.f)) * rot.rot;
    });
}

void up::Scene::flush() {
    _transformQuery.select(*_world, [&](components::Rotation const& rot, components::Position const& pos, components::Transform& trans) {
        trans.trans = glm::translate(pos.xyz) * glm::mat4_cast(rot.rot);
    });
}

void up::Scene::render(RenderContext& ctx) {
    _renderableMeshQuery.select(*_world, [&](components::Mesh& mesh, components::Transform const& trans) {
        mesh.model->render(ctx, trans.trans);
    });
}

auto up::Scene::load(Stream file) -> bool {
    nlohmann::json doc;
    if (readJson(file, doc) != IOResult::Success) {
        return false;
    }

    return false;
}

void up::Scene::save(Stream file) {
    auto doc = nlohmann::json::object();

    reflex::JsonStreamSerializer serializer(doc);
}
