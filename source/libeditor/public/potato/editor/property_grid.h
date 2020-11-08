// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/reflex/schema.h"
#include "potato/spud/concepts.h"
#include "potato/spud/string.h"

#include <glm/fwd.hpp>

namespace up::inline editor {
    class PropertyGrid {
    public:
        bool beginItem(char const* label);
        void endItem();

        void editObjectRaw(reflex::Schema const& schema, void* object) { _editProperties(schema, object); }

        template <typename T>
        void editObject(T const& value) {
            editObjectRaw(reflex::getSchema<T>(), &value);
        }

    private:
        bool _beginProperty(reflex::SchemaField const& field, void* object);
        void _endProperty();

        void _editProperties(reflex::Schema const& schema, void* object);
        void _editProperty(reflex::SchemaField const& field, void* object);

        void _drawEditor(reflex::Schema const& schema, void* object);
        void _drawObjectEditor(reflex::Schema const& schema, void* object);
        void _drawArrayEditor(reflex::Schema const& schema, void* object);

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
    };
} // namespace up::inline editor
