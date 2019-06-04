// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#include "_export.h"
#include <potato/foundation/zstring_view.h>
#include <potato/foundation/traits.h>

namespace up::reflex {

    // This isn't a real class, but more of a "guideline" since C++ doesn't have
    // the right capabilities yet to express this kind of type requirement.
    //
    class Serializer {
    public:
        template <typename Class, typename Type>
        void field(zstring_view name, Class& object, Type Class::*);

        template <typename Class, typename ReturnType, typename... ArgTypes>
        void function(zstring_view name, Class& object, ReturnType (Class::*function)(ArgTypes...));

        template <typename Class, typename ReturnType, typename... ArgTypes>
        void function(zstring_view name, Class& object, ReturnType (Class::*function)(ArgTypes...) const);

        template <typename Type>
        void value(Type& value);
    };

    // Creates the "public" interface inside of a reflect::serialize_value function
    //
    template <typename Type, typename Serializer, bool IsClassType>
    class SerializerWrapper {
    public:
        constexpr SerializerWrapper(Type& object, Serializer& serializer) noexcept : _object(object), _serializer(serializer) {}

        constexpr auto operator()() {
            return _serializer.value(_object);
        }

    protected:
        Type& _object;
        Serializer& _serializer;
    };

    template <typename Type, typename Serializer>
    class SerializerWrapper<Type, Serializer, true> final : public SerializerWrapper<Type, Serializer, false>{
    public:
        using SerializerWrapper<Type, Serializer, false>::SerializerWrapper;
        using SerializerWrapper<Type, Serializer, false>::operator();

        template <typename FieldType>
        constexpr auto operator()(zstring_view name, FieldType Type::*field) {
            return this->_serializer.field(name, this->_object, field);
        }

        template <typename ReturnType, typename... ArgTypes>
        constexpr auto operator()(zstring_view name, ReturnType (Type::*function)(ArgTypes...)) {
            return this->_serializer.function(name, this->_object, function);
        }

        template <typename ReturnType, typename... ArgTypes>
        constexpr auto operator()(zstring_view name, ReturnType (Type::*function)(ArgTypes...) const) {
            return this->_serializer.function(name, this->_object, function);
        }
    };

    template <typename Type, typename Reflector, bool IsClass>
    class ReflectorWrapper {
    public:
        constexpr ReflectorWrapper(Reflector& reflector) noexcept : _reflector(reflector) {}

        constexpr auto operator()() {
            return _reflector.value<Type>();
        }

    protected:
        Reflector& _reflector;
    };

    template <typename Type, typename Reflector>
    class ReflectorWrapper<Type, Reflector, true> final : public ReflectorWrapper<Type, Reflector, false> {
    public:
        using ReflectorWrapper<Type, Reflector, false>::ReflectorWrapper;
        using ReflectorWrapper<Type, Reflector, false>::operator();

        template <typename FieldType>
        constexpr auto operator()(zstring_view name, FieldType Type::*field) {
            return this->_reflector.field(name, field);
        }

        template <typename ReturnType, typename... ArgTypes>
        constexpr auto operator()(zstring_view name, ReturnType (Type::*function)(ArgTypes...)) {
            return this->_reflector.function(name, function);
        }

        template <typename ReturnType, typename... ArgTypes>
        constexpr auto operator()(zstring_view name, ReturnType (Type::*function)(ArgTypes...) const) {
            return this->_reflector.function(name, function);
        }
    };

    // This a selector for the type being serialized, which ignores const-ness
    //
    template <typename T>
    struct TypeTag { using type = T; };

    // Defines reflection for a type
    //
    #define UP_REFLECT_TYPE(T) \
        template <typename _up_ReflectObject> \
        void serialize_value(::up::reflex::TypeTag<T> tag, _up_ReflectObject& reflect)

    // Public entry for serialization
    //
    template <typename Type, typename Serializer>
    void serialize(Type& value, Serializer& serializer) {
        using BaseType = up::remove_cvref_t<Type>;
        ::up::reflex::SerializerWrapper<Type, Serializer, std::is_class_v<BaseType>> wrapper(value, serializer);
        serialize_value(TypeTag<BaseType>{}, wrapper);
    }

    // Public entry for reflection
    template <typename Type, typename Reflector>
    void reflect(Reflector& reflector) {
        ::up::reflex::ReflectorWrapper<Type, Reflector, std::is_class_v<Type>> wrapper(reflector);
        serialize_value(TypeTag<Type>{}, wrapper);
    }

    UP_REFLECT_TYPE(int) { reflect(); }
    UP_REFLECT_TYPE(float) { reflect(); }
    UP_REFLECT_TYPE(double) { reflect(); }

    UP_REFLECT_TYPE(string_view) { reflect(); }
    
    UP_REFLECT_API auto example(int x) noexcept -> int;
} // namespace up
