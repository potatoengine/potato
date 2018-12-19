// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "assert.h"
#include <type_traits>

namespace gm {
    template <typename Signature>
    class delegate;
}

namespace gm::_detail {
    static constexpr size_t kDelegateSize = 3;

    template <typename R, typename... P>
    struct delegate_vtable {
        void (*move)(void* dst, void* src) = nullptr;
        void (*destruct)(void* obj) = nullptr;
        R(*call)
        (void* obj, P&&... params) = nullptr;

        constexpr explicit operator bool() const { return move != nullptr; }
    };

    template <typename R>
    struct delegate_call_helper {
        template <typename F, typename... P>
        static R call(F&& f, P&&... p) { return std::invoke(std::forward<F>(f), std::forward<P>(p)...); }
    };
    template <>
    struct delegate_call_helper<void> {
        template <typename F, typename... P>
        static void call(F&& f, P&&... p) { std::invoke(std::forward<F>(f), std::forward<P>(p)...); }
    };

    template <typename F>
    void delegate_move(void* dst, void* src) { new (dst) F(std::move(*static_cast<F*>(src))); }
    template <typename F>
    void delegate_destruct(void* obj) { static_cast<F*>(obj)->~F(); }
    template <typename F, typename R, typename... P>
    R delegate_call(void* obj, P&&... params) { return delegate_call_helper<R>::call(*static_cast<F*>(obj), std::forward<P>(params)...); }

    template <typename F, typename R, typename... P>
    constexpr delegate_vtable<R, P...> vtable = {delegate_move<F>, delegate_destruct<F>, delegate_call<F, R, P...>};
} // namespace gm::_detail

/// <summary> Wrapper for a function object, analogous to std::function. </summary>
/// <typeparam name="ReturnType"> Type of the return type. </typeparam>
/// <typeparam name="ParamTypes"> Type of the parameter types. </typeparam>
template <typename ReturnType, typename... ParamTypes>
class gm::delegate<ReturnType(ParamTypes...)> {
    using VTable = _detail::delegate_vtable<ReturnType, ParamTypes...>;
    using Storage = std::aligned_storage_t<_detail::kDelegateSize * sizeof(void*), std::alignment_of<double>::value>;

public:
    constexpr delegate() = default;
    ~delegate() { reset(); }

    delegate(delegate&& rhs) { *this = std::move(rhs); }
    inline delegate& operator=(delegate&& rhs);

    delegate(const delegate&) = delete;
    delegate& operator=(const delegate&) = delete;

    /// <summary> Construct a new delegate from a function object, such as a lambda or function pointer. </summary>
    /// <param name="function"> The function to bind. </param>
    template <typename Functor, typename = std::enable_if_t<std::is_invocable_v<Functor, ParamTypes...> && !std::is_same_v<std::decay_t<Functor>, delegate>>>
    /*implicit*/ delegate(Functor&& function) { *this = std::forward<Functor>(function); }
    template <typename Functor, typename = std::enable_if_t<std::is_invocable_v<Functor, ParamTypes...> && !std::is_same_v<std::decay_t<Functor>, delegate>>>
    inline delegate& operator=(Functor&& function);

    /*implicit*/ delegate(std::nullptr_t) {}
    delegate& operator=(std::nullptr_t) {
        reset();
        return *this;
    }

    /// <summary> Invoke the bound delegate, which must not be empty. </summary>
    /// <param name="params"> Function arguments. </param>
    /// <returns> The return value of the bound function, if any. </returns>
    inline ReturnType operator()(ParamTypes... params);

    /// <summary> Check if the delegate is currently bound to a function. </summary>
    /// <returns> True if a delegate is bound. </returns>
    explicit operator bool() const { return !!_vtable; }

    /// <summary> Check if the delegate is not bound to a function. </summary>
    /// <returns> True if no delegate. </returns>
    bool empty() const { return _vtable == nullptr; }

    inline void reset(std::nullptr_t = nullptr);

    bool operator!=(std::nullptr_t) const { return !!_vtable; }
    bool operator==(std::nullptr_t) const { return !_vtable; }

private:
    // we will overwrite this with an object with just a vtable - if we are nullptr, we have no real vtable
    VTable const* _vtable = nullptr;
    Storage _storage;
};

template <typename ReturnType, typename... ParamTypes>
auto gm::delegate<ReturnType(ParamTypes...)>::operator=(delegate&& rhs) -> delegate& {
    if (this != &rhs) {
        if (_vtable != nullptr)
            _vtable->destruct(&_storage);

        _vtable = rhs._vtable;

        if (rhs._vtable != nullptr) {
            rhs._vtable->move(&_storage, &rhs._storage);
            rhs._vtable->destruct(&rhs._storage);
            rhs._vtable = nullptr;
        }
    }

    return *this;
}

template <typename ReturnType, typename... ParamTypes>
template <typename Functor, typename>
auto gm::delegate<ReturnType(ParamTypes...)>::operator=(Functor&& functor) -> delegate& {
    using FunctorType = std::decay_t<Functor>;

    static_assert(alignof(FunctorType) <= alignof(Storage), "Alignment of the functor given to delegate is too strict");
    static_assert(sizeof(FunctorType) <= sizeof(Storage), "Size of the functor given to delegate is too wide");

    if (_vtable != nullptr)
        _vtable->destruct(&_storage);

    _vtable = &_detail::vtable<FunctorType, ReturnType, ParamTypes...>;
    new (&_storage) FunctorType(std::forward<Functor>(functor));

    return *this;
}

template <typename ReturnType, typename... ParamTypes>
ReturnType gm::delegate<ReturnType(ParamTypes...)>::operator()(ParamTypes... params) {
    GM_ASSERT(_vtable != nullptr, "Invoking an empty delegate");
    return _vtable->call(&_storage, std::forward<ParamTypes>(params)...);
}

template <typename ReturnType, typename... ParamTypes>
void gm::delegate<ReturnType(ParamTypes...)>::reset(std::nullptr_t) {
    if (_vtable != nullptr) {
        _vtable->destruct(&_storage);
        _vtable = nullptr;
    }
}

namespace gm {
    template <typename ReturnType, typename... ParamTypes>
    delegate<ReturnType(ParamTypes...)> bind(ReturnType (*function)(ParamTypes...)) {
        return delegate<ReturnType(ParamTypes...)>([function](ParamTypes&&... params) { return function(std::forward<ParamTypes>(params)...); });
    }

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate<ReturnType(ParamTypes...)> bind(ClassType& object, ReturnType (ClassType::*method)(ParamTypes...)) {
        return delegate<ReturnType(ParamTypes...)>([&object, method](ParamTypes&&... params) { return (object.*method)(std::forward<ParamTypes>(params)...); });
    }

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate<ReturnType(ParamTypes...)> bind(ClassType const& object, ReturnType (ClassType::*method)(ParamTypes...) const) {
        return delegate<ReturnType(ParamTypes...)>([&object, method](ParamTypes&&... params) { return (object.*method)(std::forward<ParamTypes>(params)...); });
    }

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate<ReturnType(ParamTypes...)> bind(ClassType&& object, ReturnType (ClassType::*method)(ParamTypes...) const) {
        return delegate<ReturnType(ParamTypes...)>([object = std::forward<ClassType>(object), method](ParamTypes&&... params) { return (object.*method)(std::forward<ParamTypes>(params)...); });
    }

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate<ReturnType(ParamTypes...)> bind(ClassType* object, ReturnType (ClassType::*method)(ParamTypes...)) {
        return delegate<ReturnType(ParamTypes...)>([object, method](ParamTypes&&... params) { return (object->*method)(std::forward<ParamTypes>(params)...); });
    }

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate<ReturnType(ParamTypes...)> bind(ClassType const* object, ReturnType (ClassType::*method)(ParamTypes...) const) {
        return delegate<ReturnType(ParamTypes...)>([object, method](ParamTypes&&... params) { return (object->*method)(std::forward<ParamTypes>(params)...); });
    }
} // namespace gm
