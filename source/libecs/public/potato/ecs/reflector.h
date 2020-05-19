// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "common.h"
#include <potato/reflex/reflect.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace up {
    class ComponentReflector {
    public:
        virtual ~ComponentReflector() = default;

        template <typename ObjectType, typename ClassType, typename FieldType, typename Annotations>
        constexpr void field(zstring_view name, ObjectType& object, FieldType ClassType::*field, Annotations&& annotations) {
            onField(name);
            value(object.*field);
        }

        template <typename ObjectType, typename FieldType, typename Annotations>
        constexpr void field(zstring_view name, ObjectType& object, FieldType&& field, Annotations&& annotations) {
            onField(name);
            value(field);
        }

        template <typename ValueType>
        void value(ValueType&& value) {
            onValue(value);
        }

    protected:
        virtual void onField(zstring_view name) = 0;
        virtual void onValue(EntityId value) = 0;
        virtual void onValue(int& value) = 0;
        virtual void onValue(float& value) = 0;
        virtual void onValue(glm::vec3& value) = 0;
        virtual void onValue(glm::quat& value) = 0;
        virtual void onValue(glm::mat4x4& value) = 0;
    };
} // namespace up
