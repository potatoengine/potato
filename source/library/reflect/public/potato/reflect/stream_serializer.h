// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#pragma once

#include "traits.h"

namespace up::reflex {
    template <typename DerivedType>
    class SerializerBase {
    public:
        enum class Action {
            Enter,
            Skip
        };

        template <typename ObjectType, typename ClassType, typename FieldType>
        constexpr void field(zstring_view name, ObjectType& object, FieldType ClassType::*field) {
            if (static_cast<DerivedType*>(this)->enterField(name) == Action::Enter) {
                value(object.*field);
                static_cast<DerivedType*>(this)->leaveField();
            }
        }

        template <typename ValueType>
        void value(ValueType&& value) {
            using Type = up::remove_cvref_t<ValueType>;
            if constexpr (is_numeric_v<Type> || is_string_v<Type>) {
                static_cast<DerivedType*>(this)->handle(value);
            }
            else if constexpr (std::is_class_v<Type>) {
                if (static_cast<DerivedType*>(this)->enterObject() == Action::Enter) {
                    serialize(value, *static_cast<DerivedType*>(this));
                    static_cast<DerivedType*>(this)->leaveObject();
                }
            }
            else {
                static_cast<DerivedType*>(this)->value(value);
            }
        }
    };
} // namespace up::reflex
