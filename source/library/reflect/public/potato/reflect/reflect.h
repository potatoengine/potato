// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#include "_export.h"
#include "_wrapper.h"
#include <potato/foundation/zstring_view.h>
#include <potato/foundation/traits.h>

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

    namespace _detail {
        // This a selector for the type being serialized, which ignores const-ness
        //
        template <typename T>
        struct TypeTag { using type = T; };
    } // namespace _detail

    // Defines reflection for a type
    //
    #define UP_REFLECT_TYPE(T) \
        template <typename _up_ReflectObject> \
        void serialize_value(::up::reflex::_detail::TypeTag<T> tag, _up_ReflectObject& reflect, ::up::zstring_view name = #T)

    UP_REFLECT_TYPE(int) { reflect(); }
    UP_REFLECT_TYPE(float) { reflect(); }
    UP_REFLECT_TYPE(double) { reflect(); }

    UP_REFLECT_TYPE(string_view) { reflect(); }

    // Public entry for serialization
    //
    template <typename Type, typename Serializer>
    void serialize(Type& value, Serializer& serializer) {
        using BaseType = up::remove_cvref_t<Type>;
        using WrapperType = _detail::SerializerWrapper<Type, Serializer, std::is_class_v<BaseType>>;
        using TagType = _detail::TypeTag<BaseType>;

        auto wrapper = WrapperType(value, serializer);
        serialize_value<WrapperType>(TagType{}, wrapper);
    }

    // Public entry for reflection
    template <typename Type, typename Reflector>
    void reflect(Reflector& reflector) {
        using WrapperType = _detail::ReflectorWrapper<Type, Reflector, std::is_class_v<Type>>;
        using TagType = _detail::TypeTag<Type>;

        auto wrapper = WrapperType(reflector);
        serialize_value<WrapperType>(TagType{}, wrapper);
    }
    
    UP_REFLECT_API auto example(int x) noexcept -> int;
} // namespace up
