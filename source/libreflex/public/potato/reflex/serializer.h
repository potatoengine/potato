// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "traits.h"

#include "potato/spud/utility.h"
#include "potato/spud/zstring_view.h"

namespace up::reflex {
    template <typename DerivedType> class SerializerBase {
    public:
        enum class Action { Enter, Skip };

        template <typename ObjectType, typename ClassType, typename FieldType, typename Annotations>
        constexpr void field(zstring_view name, ObjectType& object, FieldType ClassType::*field, Annotations&& annotations) {
            using Type = remove_cvref_t<FieldType>;
            if (static_cast<DerivedType*>(this)->enterField(tag<Type>{}, name, std::forward<Annotations>(annotations)) == Action::Enter) {
                value(object.*field);
            }
        }

        template <typename ObjectType, typename FieldType, typename Annotations>
        constexpr void field(zstring_view name, ObjectType& object, FieldType&& field, Annotations&& annotations) {
            using Type = remove_cvref_t<FieldType>;
            if (static_cast<DerivedType*>(this)->enterField(tag<Type>{}, name, std::forward<Annotations>(annotations)) == Action::Enter) {
                value(field);
            }
        }

        template <typename ValueType> void value(ValueType&& value) {
            using Type = remove_cvref_t<ValueType>;
            if constexpr (is_vector_v<Type>) {
                size_t size = value.size();
                if (static_cast<DerivedType*>(this)->beginArray(tag<Type>{}, size) == Action::Enter) {
                    if constexpr (!std::is_const_v<std::remove_reference_t<ValueType>>) {
                        value.resize(size);
                    }
                    size_t index = 0;
                    for (auto&& item : value) {
                        if (static_cast<DerivedType*>(this)->enterItem(tag<decltype(item)>{}, index++) == Action::Enter) {
                            serialize(item, *static_cast<DerivedType*>(this));
                        }
                    }
                    static_cast<DerivedType*>(this)->end();
                }
            }
            else if constexpr (is_numeric_v<Type> || is_string_v<Type>) {
                static_cast<DerivedType*>(this)->handle(value);
            }
            else if constexpr (std::is_class_v<Type>) {
                if (static_cast<DerivedType*>(this)->beginObject(tag<Type>{}) == Action::Enter) {
                    serialize(value, *static_cast<DerivedType*>(this));
                    static_cast<DerivedType*>(this)->end();
                }
            }
        }
    };
} // namespace up::reflex
