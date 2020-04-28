// Copyright (C) 2019,2020 Sean Middleditch, all rights reserverd.

#include <potato/ecs/component.h>
#include <potato/render/model.h>
#include <potato/reflex/reflect.h>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

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

    UP_REFLECT_TYPE(Position) {
        reflect("xyz", &Position::xyz);
    }
    UP_REFLECT_TYPE(Rotation) {
        reflect("rotation", &Rotation::rot);
    }
    UP_REFLECT_TYPE(Transform) {}
    UP_REFLECT_TYPE(Mesh) {}
    UP_REFLECT_TYPE(Wave) {
        reflect("time", &Wave::time);
        reflect("offset", &Wave::offset);
    }
    UP_REFLECT_TYPE(Spin) {
        reflect("radians", &Spin::radians);
    }
} // namespace up::components

UP_DECLARE_COMPONENT(up::components::Position);
UP_DECLARE_COMPONENT(up::components::Rotation);
UP_DECLARE_COMPONENT(up::components::Transform);
UP_DECLARE_COMPONENT(up::components::Mesh);
UP_DECLARE_COMPONENT(up::components::Wave);
UP_DECLARE_COMPONENT(up::components::Spin);
