// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#pragma once

#include "_export.h"
#include "_wrapper.h"
#include "metadata.h"
#include "traits.h"
#include <potato/spud/zstring_view.h>
#include <potato/spud/string.h>
#include <potato/spud/traits.h>
#include <potato/spud/utility.h>

namespace up::reflex {

    // This isn't a real class, but more of a "guideline" since C++ doesn't have
    // the right capabilities yet to express this kind of type requirement.
    //
    // class Serializer {
    // public:
    //     template <typename Class, typename Type>
    //     void field(zstring_view name, Class& object, Type Class::*);
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

    /// Defines reflection for a type
    #define UP_REFLECT_TYPE(T) \
        constexpr auto getTypeInfo(::up::tag<up::remove_cvref_t<T>>) noexcept->::up::reflex::TypeInfo { \
            return ::up::reflex::TypeInfo{#T, sizeof(up::remove_cvref_t<T>), alignof(up::remove_cvref_t<T>)}; \
        } \
        template <typename _up_ReflectObject> \
        void serialize_value(::up::tag<up::remove_cvref_t<T>>, _up_ReflectObject& reflect, ::up::zstring_view name = #T)

    UP_REFLECT_TYPE(int) { reflect(); }
    UP_REFLECT_TYPE(unsigned) { reflect(); }
    UP_REFLECT_TYPE(size_t) { reflect(); }
    UP_REFLECT_TYPE(float) { reflect(); }
    UP_REFLECT_TYPE(double) { reflect(); }

    UP_REFLECT_TYPE(string_view) { reflect(); }
    UP_REFLECT_TYPE(string&) { reflect(); }

    /// Public entry for serialization
    template <typename Type, typename Serializer>
    void serialize(Type& value, Serializer& serializer) {
        using BaseType = up::remove_cvref_t<Type>;
        using WrapperType = _detail::SerializerWrapper<Type, Serializer, std::is_class_v<BaseType>>;
        using TagType = tag<BaseType>;

        auto wrapper = WrapperType(value, serializer);
        serialize_value<WrapperType>(TagType{}, wrapper);
    }

    /// Public entry for reflection
    template <typename Type, typename Reflector>
    void reflect(Reflector& reflector) {
        using WrapperType = _detail::ReflectorWrapper<Type, Reflector, std::is_class_v<Type>>;
        using TagType = tag<Type>;

        auto wrapper = WrapperType(reflector);
        serialize_value<WrapperType>(TagType{}, wrapper);
    }
} // namespace up
