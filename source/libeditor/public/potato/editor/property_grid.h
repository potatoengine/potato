// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/reflex/schema.h"
#include "potato/spud/concepts.h"
#include "potato/spud/string.h"

#include <glm/fwd.hpp>

namespace up::inline editor {
    class PropertyGrid {
    public:
        void drawGridRaw(zstring_view name, reflex::Schema const& schema, void* object);

        template <typename T>
        void drawGrid(zstring_view name, T const& value) {
            drawGridRaw(name, reflex::getSchema<T>(), &value);
        }

        void drawPropertyRaw(reflex::SchemaField const& field, void* object);

        void drawEditor(reflex::Schema const& schema, void* object);
        void drawObjectEditor(reflex::Schema const& schema, void* object);
        void drawArrayEditor(reflex::Schema const& schema, void* object);

        void drawIntEditor(int& value) noexcept;
        template <integral T>
        void drawIntEditor(T& value) noexcept {
            int tmp = static_cast<int>(value);
            drawIntEditor(tmp);
            value = static_cast<T>(tmp);
        }
        void drawFloatEditor(float& value) noexcept;
        void drawFloatEditor(double& value) noexcept;
        void drawVec3Editor(glm::vec3& value) noexcept;
        void drawMat4x4Editor(glm::mat4x4& value) noexcept;
        void drawQuatEditor(glm::quat& value) noexcept;

        void drawStringEditor(string& value) noexcept;

        template <integral T>
        void drawIntEditor(zstring_view name, T& value) noexcept {
            int tmp = value;
            drawIntEditor(name, tmp);
            value = tmp;
        }
    };
} // namespace up::inline editor
