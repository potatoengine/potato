// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "scene.h"
#include "components.h"
#include <potato/runtime/json.h>
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

up::Scene::Scene(Universe& universe) : _world{universe.createWorld()},
                                       _waveQuery{universe.createQuery<components::Position, components::Wave>()},
                                       _orbitQuery{universe.createQuery<components::Position>()},
                                       _spinQuery{universe.createQuery<components::Rotation, components::Spin>()},
                                       _transformQuery{universe.createQuery<components::Rotation, components::Position, components::Transform>()},
                                       _renderableMeshQuery{universe.createQuery<components::Mesh, components::Transform>()} {}

void up::Scene::create(rc<Model> const& cube) {
    auto pi = glm::pi<float>();

    for (size_t i = 0; i <= 100; ++i) {
        float p = i / 100.0f;
        float r = p * 2.f * pi;
        _world.createEntity(
            components::Position{{(20 + glm::cos(r) * 10.f) * glm::sin(r),
                                  1 + glm::sin(r * 10.f) * 5.f,
                                  (20 + glm::sin(r) * 10.f) * glm::cos(r)}},
            components::Rotation{glm::identity<glm::quat>()},
            components::Transform{},
            components::Mesh{cube},
            components::Wave{0, r},
            components::Spin{glm::sin(r) * 2.f - 1.f});
    }

    _main = _world.createEntity(
        components::Position{{0, 5, 0}},
        components::Rotation{glm::identity<glm::quat>()},
        components::Transform(),
        components::Mesh{cube});
}

up::Scene::~Scene() {
    _cube.reset();
}

void up::Scene::tick(float frameTime) {
    if (!_playing) {
        return;
    }

    _waveQuery.select(_world, [&](EntityId, components::Position& pos, components::Wave& wave) {
        wave.offset += frameTime * .2f;
        pos.xyz.y = 1 + 5 * glm::sin(wave.offset * 10);
    });

    _orbitQuery.select(_world, [&](EntityId, components::Position& pos) {
        pos.xyz = glm::rotateY(pos.xyz, frameTime);
    });

    _spinQuery.select(_world, [&](EntityId, components::Rotation& rot, components::Spin const& spin) {
        rot.rot = glm::angleAxis(spin.radians * frameTime, glm::vec3(0.f, 1.f, 0.f)) * rot.rot;
    });
}

void up::Scene::flush() {
    _transformQuery.select(_world, [&](EntityId, components::Rotation const& rot, components::Position const& pos, components::Transform& trans) {
        trans.trans = glm::translate(pos.xyz) * glm::mat4_cast(rot.rot);
    });
}

void up::Scene::render(RenderContext& ctx) {
    _renderableMeshQuery.select(_world, [&](EntityId, components::Mesh& mesh, components::Transform const& trans) {
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
