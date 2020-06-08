// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "_wrapper.h"
#include "traits.h"

#include "potato/spud/string.h"
#include "potato/spud/traits.h"
#include "potato/spud/utility.h"
#include "potato/spud/zstring_view.h"

namespace up::reflex {

    // This isn't a real class, but more of a "guideline" since C++ doesn't have
    // the right capabilities yet to express this kind of type requirement.
    //
    // class Serializer {
    // public:
    //     template <typename Class, typename Type, typename... Annotations>
    //     void field(zstring_view name, Class& object, Type Class::*, Annotations&&...);
    //
    //     template <typename Class, typename ReturnType, typename... ArgTypes>
    //     void function(zstring_view name, Class& object, ReturnType (Class::*function)(ArgTypes...));
    //
    //     template <typename Class, typename ReturnType, typename... ArgTypes>
    //     void function(zstring_view name, Class& object, ReturnType (Class::*function)(ArgTypes...) const);
    //
    //     template <typename Type>
    //     void value(Type& value);
    // };

    namespace _detail {
        template <typename Reflect, typename T> void serialize_value(tag<T>, Reflect&) = delete;

        template <typename Reflect, typename T> inline void serialize_value_impl(tag<T>, Reflect& reflect) {
            if constexpr (is_numeric_v<T> || is_string_v<T> || is_vector_v<T>) {
                reflect();
            }
            else {
                serialize_value(tag<T>{}, reflect);
            }
        }
    } // namespace _detail

    /// Public entry for serialization
    template <typename Type, typename Serializer> inline void serialize(Type&& value, Serializer&& serializer) {
        using BaseType = up::remove_cvref_t<Type>;

        auto wrapper =
            _detail::SerializerWrapper<std::remove_reference_t<Type>, remove_cvref_t<Serializer>, std::is_class_v<BaseType>>(value, serializer);
        _detail::serialize_value_impl<decltype(wrapper)>(tag<BaseType>{}, wrapper);
    }

    /// Public entry for reflection
    template <typename Type, typename Reflector> inline void reflect(Reflector&& reflector) {
        using BaseType = up::remove_cvref_t<Type>;

        auto wrapper = _detail::ReflectorWrapper<std::remove_reference_t<Type>, remove_cvref_t<Reflector>, std::is_class_v<BaseType>>(reflector);
        _detail::serialize_value_impl<decltype(wrapper)>(tag<BaseType>{}, wrapper);
    }
} // namespace up::reflex

/// Defines reflection for a type
#define UP_REFLECT_TYPE(T) \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses) */ \
    constexpr auto typeName [[maybe_unused]] (::up::tag<up::remove_cvref_t<T>>) noexcept->::up::zstring_view { return (#T); } \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses) */ \
    template <typename _up_ReflectObject> inline void serialize_value(::up::tag<T>, _up_ReflectObject& reflect)
