// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "scene.h"
#include "components_schema.h"

#include "potato/audio/audio_engine.h"
#include "potato/ecs/query.h"
#include "potato/ecs/world.h"
#include "potato/reflex/json_reflex_serializer.h"
#include "potato/reflex/reflect.h"
#include "potato/render/model.h"
#include "potato/runtime/json.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>

up::Scene::Scene(Universe& universe, AudioEngine& audioEngine)
    : _audioEngine(audioEngine)
    , _world{universe.createWorld()}
    , _waveQuery{universe.createQuery<components::Transform, components::Wave>()}
    , _orbitQuery{universe.createQuery<components::Transform>()}
    , _spinQuery{universe.createQuery<components::Transform, components::Spin>()}
    , _dingQuery{universe.createQuery<components::Ding>()}
    , _transformQuery{universe.createQuery<components::Transform>()}
    , _renderableMeshQuery{universe.createQuery<components::Mesh, components::Transform>()} {}

void up::Scene::create(rc<Model> const& cube, rc<SoundResource> const& ding) {
    auto pi = glm::pi<float>();

    constexpr int numObjects = 100;

    for (size_t i = 0; i <= numObjects; ++i) {
        float p = i / static_cast<float>(numObjects);
        float r = p * 2.f * pi;
        _world.createEntity(
            components::Transform{
                .position =
                    {(20 + glm::cos(r) * 10.f) * glm::sin(r),
                     1 + glm::sin(r * 10.f) * 5.f,
                     (20 + glm::sin(r) * 10.f) * glm::cos(r)},
                .rotation = glm::identity<glm::quat>()},
            components::Transform{},
            components::Mesh{cube},
            components::Wave{0, r},
            components::Spin{glm::sin(r) * 2.f - 1.f});
    }

    _root = _world.createEntity(
        components::Transform{.position = {0, 5, 0}, .rotation = glm::identity<glm::quat>()},
        components::Mesh{cube},
        components::Ding{2, 0, ding});
}

up::Scene::~Scene() {
    _cube.reset();
}

void up::Scene::tick(float frameTime) {
    if (!_playing) {
        return;
    }

    _waveQuery.select(_world, [&](EntityId, components::Transform& trans, components::Wave& wave) {
        wave.offset += frameTime * .2f;
        trans.position.y = 1 + 5 * glm::sin(wave.offset * 10);
    });

    _orbitQuery.select(_world, [&](EntityId, components::Transform& trans) {
        trans.position = glm::rotateY(trans.position, frameTime);
    });

    _spinQuery.select(_world, [&](EntityId, components::Transform& trans, components::Spin const& spin) {
        trans.rotation = glm::angleAxis(spin.radians * frameTime, glm::vec3(0.f, 1.f, 0.f)) * trans.rotation;
    });

    _dingQuery.select(_world, [&, this](EntityId, components::Ding& ding) {
        ding.time += frameTime;
        if (ding.time > ding.period) {
            ding.time -= ding.period;
            _audioEngine.play(ding.sound.get());
        }
    });
}

void up::Scene::flush() {
    _transformQuery.select(_world, [&](EntityId, components::Transform& trans) {
        trans.transform = glm::translate(trans.position) * glm::mat4_cast(trans.rotation);
    });
}

void up::Scene::render(RenderContext& ctx) {
    _renderableMeshQuery.select(_world, [&](EntityId, components::Mesh& mesh, components::Transform const& trans) {
        mesh.model->render(ctx, trans.transform);
    });
}

auto up::Scene::load(Stream file) -> bool {
    if (auto [rs, doc] = readJson(file); rs == IOResult::Success) {
        return false;
    }

    return false;
}

void up::Scene::save(Stream file) {
    auto doc = nlohmann::json::object();

    reflex::JsonStreamSerializer serializer(doc);
}
