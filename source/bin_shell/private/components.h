// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/ecs/component.h"
#include "potato/reflex/reflect.h"
#include "potato/render/model.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

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

    UP_REFLECT_TYPE(Position) { reflect("xyz", &Position::xyz); }
    UP_REFLECT_TYPE(Rotation) { reflect("rotation", &Rotation::rot); }
    UP_REFLECT_TYPE(Transform) { reflect("matrix", &Transform::trans); }
    UP_REFLECT_TYPE(Mesh) {}
    UP_REFLECT_TYPE(Wave) {
        reflect("time", &Wave::time);
        reflect("offset", &Wave::offset);
    }
    UP_REFLECT_TYPE(Spin) { reflect("radians", &Spin::radians); }
} // namespace up::components
