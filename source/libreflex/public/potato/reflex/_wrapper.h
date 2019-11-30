// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#pragma once

#include <potato/spud/zstring_view.h>
#include <potato/spud/traits.h>

namespace up::reflex::_detail {
    template <typename T, typename Getter, typename Setter>
    struct Binding {
        using Type = T;

        Getter getter;
        Setter setter;
    };

    template <typename T>
    struct ReflectTypeHelper {
        using Type = T;
        constexpr static bool IsBinding = false;
    };

    template <typename T, typename G, typename S>
    struct ReflectTypeHelper<Binding<T, G, S>> {
        using Type = T;
        constexpr static bool IsBinding = true;
    };

    template <typename T>
    using ReflectType = typename ReflectTypeHelper<T>::Type;

    template <typename T>
    constexpr bool IsReflectBinding = ReflectTypeHelper<T>::IsBinding;

    // Creates the "public" interface inside of a reflect::serialize_value function
    //
    template <typename Type, typename Serializer, bool IsClassType>
    class SerializerWrapper {
    public:
        constexpr SerializerWrapper(Type& object, Serializer& serializer) noexcept : _object(object), _serializer(serializer) {}

        SerializerWrapper(SerializerWrapper const&) = delete;
        SerializerWrapper& operator=(SerializerWrapper const&) = delete;

        constexpr auto operator()() {
            return _serializer.value(_object);
        }

        template <typename Getter, typename Setter>
        constexpr auto operator()(zstring_view name, Getter getter, Setter setter) {
            using ValueType = decltype(getter(this->_object));
            auto g = [&object = _object, &getter]() -> decltype(auto) { return getter(object); };
            auto s = [&object = _object, &setter](ValueType const& value) mutable {
                setter(const_cast<std::remove_const_t<Type>*>(&object), value);
            };
            Binding<ValueType, decltype(g), decltype(s)> binding = {g, s};
            return this->_serializer.field(name, this->_object, binding);
        }

    protected:
        Type& _object;
        Serializer& _serializer;
    };

    template <typename Type, typename Serializer>
    class SerializerWrapper<Type, Serializer, true> final : public SerializerWrapper<Type, Serializer, false> {
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

    // Creates the "public" interface inside of a reflect::serialize_value function
    //
    template <typename Type, typename Reflector, bool IsClass>
    class ReflectorWrapper {
    public:
        constexpr ReflectorWrapper(Reflector& reflector) noexcept : _reflector(reflector) {}

        ReflectorWrapper(ReflectorWrapper const&) = delete;
        ReflectorWrapper& operator=(ReflectorWrapper const&) = delete;

        constexpr auto operator()() {
            return _reflector.template value<Type>();
        }

        template <typename LambdaType>
        constexpr auto operator()(zstring_view name, LambdaType const& lambda) {
            using ValueType = decltype(lambda(this->_object));
            return _reflector.template value<ValueType>();
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
} // namespace up::reflex::_detail
