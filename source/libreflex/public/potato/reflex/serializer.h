// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#pragma once

#include "traits.h"
#include "_wrapper.h"

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
            using Type = _detail::ReflectType<FieldType>;
            if (static_cast<DerivedType*>(this)->enterField(tag<Type>{}, name) == Action::Enter) {
                value(object.*field);
                static_cast<DerivedType*>(this)->leaveField(tag<Type>{}, name);
            }
        }

        template <typename ObjectType, typename FieldType>
        constexpr void field(zstring_view name, ObjectType& object, FieldType& field) {
            using Type = _detail::ReflectType<FieldType>;
            if (static_cast<DerivedType*>(this)->enterField(tag<Type>{}, name) == Action::Enter) {
                value(field);
                static_cast<DerivedType*>(this)->leaveField(tag<Type>{}, name);
            }
        }

        template <typename ValueType>
        void value(ValueType&& value) {
            using Type = _detail::ReflectType<ValueType>;
            if constexpr (_detail::IsReflectBinding<ValueType>) {
                static_cast<DerivedType*>(this)->dispatch(tag<Type>{}, value.getter, value.setter);
            }
            else if constexpr (is_numeric_v<Type> || is_string_v<Type>) {
                static_cast<DerivedType*>(this)->handle(value);
            }
            else if constexpr (std::is_class_v<Type>) {
                if (static_cast<DerivedType*>(this)->enterObject(tag<Type>{}) == Action::Enter) {
                    serialize(value, *static_cast<DerivedType*>(this));
                    static_cast<DerivedType*>(this)->leaveObject(tag<Type>{});
                }
            }
            else {
                static_cast<DerivedType*>(this)->value(value);
            }
        }
    };
} // namespace up::reflex
