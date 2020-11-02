// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/reflex/schema.h"
#include "potato/spud/concepts.h"

#include <glm/fwd.hpp>

namespace up::shell {
    class PropertyGrid {
    public:
        void drawGridRaw(zstring_view name, reflex::Schema const& schema, void* object);

        template <typename T>
        void drawGrid(zstring_view name, T const& value)
        {
            drawGridRaw(name, getSchema<T>(), &value);
        }

        void drawPropertyRaw(reflex::SchemaField const& field, void* object);

        void drawIntEditor(zstring_view name, int& value) noexcept;
        void drawFloatEditor(zstring_view name, float& value) noexcept;
        void drawVec3Editor(zstring_view name, glm::vec3& value) noexcept;
        void drawMat4x4Editor(zstring_view name, glm::mat4x4& value) noexcept;
        void drawQuatEditor(zstring_view name, glm::quat& value) noexcept;

        template <integral T>
        void drawIntEditor(zstring_view name, T& value) noexcept {
            int tmp = value;
            drawIntEditor(name, tmp);
            value = tmp;
        }
    };
}
