// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#pragma once

#include "_export.h"
#include "metadata.h"

// temporary - FIXME remove
namespace up {
    class string_view;
    class zstring_view;
    class string;

    template <typename T>
    constexpr bool is_string_v = std::is_same_v<T, up::string_view> || std::is_same_v<T, up::zstring_view> || std::is_same_v<T, up::string>;
} // namespace up

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
