// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "assertion.h"
#include "traits.h"

namespace gm {
    template <typename Signature>
    class delegate;

    template <typename T>
    delegate(T)->delegate<signature_t<T>>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate(ClassType& object, ReturnType (ClassType::*method)(ParamTypes...))->delegate<ReturnType(ParamTypes...)>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate(ClassType const& object, ReturnType (ClassType::*method)(ParamTypes...) const)->delegate<ReturnType(ParamTypes...) const>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate(ClassType&& object, ReturnType (ClassType::*method)(ParamTypes...) const)->delegate<ReturnType(ParamTypes...) const>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate(ClassType* object, ReturnType (ClassType::*method)(ParamTypes...))->delegate<ReturnType(ParamTypes...)>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate(ClassType const* object, ReturnType (ClassType::*method)(ParamTypes...) const)->delegate<ReturnType(ParamTypes...) const>;
} // namespace gm

namespace gm::_detail {
    static constexpr size_t delegate_size_c = 3;

    template <typename Functor, typename... ParamTypes>
    constexpr bool is_invocable_v = true;//std::is_invocable_v<Functor, ParamTypes...>;

    struct delegate_vtable_base {
        using move_t = void (*)(void* dst, void* src);
        using destruct_t = void (*)(void* obj);

        move_t move = nullptr;
        destruct_t destruct = nullptr;
    };

    template <typename R, typename... P>
    struct delegate_vtable_typed : delegate_vtable_base {
        using call_t = R (*)(void* obj, P&&... params);

        call_t call = nullptr;
    };

    template <typename F>
    struct delegate_vtable_impl {
        static void move(void* dst, void* src) { new (dst) F(std::move(*static_cast<F*>(src))); }
        static void destruct(void* obj) { static_cast<F*>(obj)->~F(); }
        template <typename R, typename... P>
        static R call(void* obj, P&&... params) {
            F& f = *static_cast<F*>(obj);
            if constexpr (std::is_same_v<R, void>) {
                std::invoke(std::forward<F>(f), std::forward<P>(params)...);
            }
            else {
                return std::invoke(std::forward<F>(f), std::forward<P>(params)...);
            }
        }
    };

    template <typename F, typename R, typename... P>
    constexpr auto vtable_c = delegate_vtable_typed<R, P...>{
        {delegate_vtable_impl<F>::move,
         delegate_vtable_impl<F>::destruct},
        delegate_vtable_impl<F>::template call<R, P...>};

    class delegate_base {
    protected:
        using storage_t = std::aligned_storage_t<_detail::delegate_size_c * sizeof(void*), std::alignment_of<double>::value>;

    public:
        delegate_base() = default;

        delegate_base(const delegate_base&) = delete;
        delegate_base& operator=(const delegate_base&) = delete;

        inline delegate_base(delegate_base&&);
        inline delegate_base& operator=(delegate_base&&);

        /*implicit*/ delegate_base(std::nullptr_t) {}
        delegate_base& operator=(std::nullptr_t) {
            reset();
            return *this;
        }

        /// <summary> Check if the delegate is currently bound to a function. </summary>
        /// <returns> True if a delegate is bound. </returns>
        explicit operator bool() const { return _vtable != nullptr; }

        /// <summary> Check if the delegate is not bound to a function. </summary>
        /// <returns> True if no delegate. </returns>
        bool empty() const { return _vtable == nullptr; }

        inline void reset(std::nullptr_t = nullptr);

        bool operator!=(std::nullptr_t) const { return _vtable != nullptr; }
        bool operator==(std::nullptr_t) const { return _vtable == nullptr; }

    protected:
        delegate_base(delegate_vtable_base const* vtable) : _vtable(vtable) {}
        ~delegate_base() = default;

    protected:
        // we will overwrite this with an object with just a vtable - if we are nullptr, we have no real vtable
        delegate_vtable_base const* _vtable = nullptr;
        storage_t _storage;
    };

    template <typename ReturnType, typename... ParamTypes>
    class delegate_typed : public delegate_base {
    protected:
        using vtable_c = _detail::delegate_vtable_typed<ReturnType, ParamTypes...>;
        using storage_t = typename delegate_base::storage_t;

    public:
        using delegate_base::delegate_base;

        /// <summary> Construct a new delegate from a function object, such as a lambda or function pointer. </summary>
        /// <param name="function"> The function to bind. </param>
        template <typename Functor, typename = enable_if_t<_detail::is_invocable_v<Functor, ParamTypes...> && !std::is_base_of_v<delegate_typed, std::decay_t<Functor>>>>
        /*implicit*/ delegate_typed(Functor&& functor);

        template <typename Functor, typename = enable_if_t<_detail::is_invocable_v<Functor, ParamTypes...> && !std::is_base_of_v<delegate_typed, std::decay_t<Functor>>>>
        delegate_typed& operator=(Functor&& functor);

    private:
        template <typename Functor>
        void assign(Functor&& functor);
    };
} // namespace gm::_detail

/// <summary> Wrapper for a function object, analogous to std::function. </summary>
/// <typeparam name="ReturnType"> Type of the return type. </typeparam>
/// <typeparam name="ParamTypes"> Type of the parameter types. </typeparam>
template <typename ReturnType, typename... ParamTypes>
class gm::delegate<ReturnType(ParamTypes...)> : public _detail::delegate_typed<ReturnType, ParamTypes...> {
    using vtable_c = _detail::delegate_vtable_typed<ReturnType, ParamTypes...>;
    using storage_t = typename _detail::delegate_typed<ReturnType, ParamTypes...>::storage_t;

public:
    using _detail::delegate_typed<ReturnType, ParamTypes...>::delegate_typed;

    template <typename ClassType>
    delegate(ClassType& object, ReturnType (ClassType::*method)(ParamTypes...))
        : delegate([&object, method](ParamTypes&&... params) { return (object.*method)(std::forward<ParamTypes>(params)...); }) {}

    template <typename ClassType>
    delegate(ClassType&& object, ReturnType (ClassType::*method)(ParamTypes...) const)
        : delegate([object = std::forward<ClassType>(object), method](ParamTypes&&... params) { return (object.*method)(std::forward<ParamTypes>(params)...); }) {}

    template <typename ClassType>
    delegate(ClassType* object, ReturnType (ClassType::*method)(ParamTypes...))
        : delegate([object, method](ParamTypes&&... params) { return (object->*method)(std::forward<ParamTypes>(params)...); }) {}

    /// <summary> Invoke the bound delegate, which must not be empty. </summary>
    /// <param name="params"> Function arguments. </param>
    /// <returns> The return value of the bound function, if any. </returns>
    inline ReturnType operator()(ParamTypes... params);
};

template <typename ReturnType, typename... ParamTypes>
class gm::delegate<ReturnType(ParamTypes...) const> : public _detail::delegate_typed<ReturnType, ParamTypes...> {
    using vtable_c = _detail::delegate_vtable_typed<ReturnType, ParamTypes...>;
    using storage_t = typename _detail::delegate_typed<ReturnType, ParamTypes...>::storage_t;

public:
    using _detail::delegate_typed<ReturnType, ParamTypes...>::delegate_typed;

    template <typename ClassType>
    delegate(ClassType const& object, ReturnType (ClassType::*method)(ParamTypes...) const)
        : delegate([&object, method](ParamTypes&&... params) { return (object.*method)(std::forward<ParamTypes>(params)...); }) {}

    template <typename ClassType>
    delegate(ClassType const* object, ReturnType (ClassType::*method)(ParamTypes...) const)
        : delegate([object, method](ParamTypes&&... params) { return (object->*method)(std::forward<ParamTypes>(params)...); }) {}

    /// <summary> Invoke the bound delegate, which must not be empty. </summary>
    /// <param name="params"> Function arguments. </param>
    /// <returns> The return value of the bound function, if any. </returns>
    inline ReturnType operator()(ParamTypes... params) const;
};

gm::_detail::delegate_base::delegate_base(delegate_base&& rhs) : _vtable(rhs._vtable) {
    if (_vtable != nullptr) {
        _vtable->move(&_storage, &rhs._storage);
        _vtable->destruct(&rhs._storage);

        rhs._vtable = nullptr;
    }
}

auto gm::_detail::delegate_base::operator=(delegate_base&& rhs) -> delegate_base& {
    if (this != &rhs) {
        if (_vtable != nullptr) {
            _vtable->destruct(&_storage);
        }

        _vtable = rhs._vtable;

        if (_vtable != nullptr) {
            _vtable->move(&_storage, &rhs._storage);
            _vtable->destruct(&rhs._storage);

            rhs._vtable = nullptr;
        }
    }

    return *this;
}

void gm::_detail::delegate_base::reset(std::nullptr_t) {
    if (_vtable != nullptr) {
        _vtable->destruct(&_storage);
        _vtable = nullptr;
    }
}

template <typename ReturnType, typename... ParamTypes>
template <typename Functor, typename>
gm::_detail::delegate_typed<ReturnType, ParamTypes...>::delegate_typed(Functor&& functor) {
    assign(std::forward<Functor>(functor));
}

template <typename ReturnType, typename... ParamTypes>
template <typename Functor, typename>
auto gm::_detail::delegate_typed<ReturnType, ParamTypes...>::operator=(Functor&& functor) -> delegate_typed& {
    if (this->_vtable != nullptr) {
        this->_vtable->destruct(&this->_storage);
    }

    assign(std::forward<Functor>(functor));
    return *this;
}

template <typename ReturnType, typename... ParamTypes>
template <typename Functor>
void gm::_detail::delegate_typed<ReturnType, ParamTypes...>::assign(Functor&& functor) {
    using FunctorType = std::decay_t<Functor>;

    static_assert(alignof(FunctorType) <= alignof(storage_t), "Alignment of the functor given to delegate is too strict");
    static_assert(sizeof(FunctorType) <= sizeof(storage_t), "Size of the functor given to delegate is too wide");

    this->_vtable = &_detail::vtable_c<FunctorType, ReturnType, ParamTypes...>;
    new (&this->_storage) FunctorType(std::forward<Functor>(functor));
}

template <typename ReturnType, typename... ParamTypes>
auto gm::delegate<ReturnType(ParamTypes...)>::operator()(ParamTypes... params) -> ReturnType {
    GM_ASSERT(this->_vtable != nullptr, "Invoking an empty delegate");
    return static_cast<vtable_c const*>(this->_vtable)->call(&this->_storage, std::forward<ParamTypes>(params)...);
}

template <typename ReturnType, typename... ParamTypes>
auto gm::delegate<ReturnType(ParamTypes...) const>::operator()(ParamTypes... params) const -> ReturnType {
    GM_ASSERT(this->_vtable != nullptr, "Invoking an empty delegate");
    return static_cast<vtable_c const*>(this->_vtable)->call(const_cast<decltype(this->_storage)*>(&this->_storage), std::forward<ParamTypes>(params)...);
}
