// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "traits.h"

namespace gm {
    template <typename Signature>
    class delegate_view;

    template <typename T>
    delegate_view(T)->delegate_view<signature_t<T>>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate_view(ClassType& object, ReturnType (ClassType::*method)(ParamTypes...))->delegate_view<ReturnType(ParamTypes...)>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate_view(ClassType const& object, ReturnType (ClassType::*method)(ParamTypes...) const)->delegate_view<ReturnType(ParamTypes...) const>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate_view(ClassType&& object, ReturnType (ClassType::*method)(ParamTypes...) const)->delegate_view<ReturnType(ParamTypes...) const>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate_view(ClassType* object, ReturnType (ClassType::*method)(ParamTypes...))->delegate_view<ReturnType(ParamTypes...)>;

    template <typename ClassType, typename ReturnType, typename... ParamTypes>
    delegate_view(ClassType const* object, ReturnType (ClassType::*method)(ParamTypes...) const)->delegate_view<ReturnType(ParamTypes...) const>;
} // namespace gm

namespace gm::_detail {
#if defined(__cpp_lib_invoke)
    using std::invoke;
#else
    template <typename Functor, typename... Args>
    constexpr decltype(auto) invoke(Functor&& func, Args&&... args) noexcept(noexcept(func(std::forward<Args>(args)...))) {
        return std::forward<Functor>(func)(std::forward<Args>(args)...);
    }
#endif

#if defined(__cpp_lib_is_invocable)
    template <typename Functor, typename... ParamTypes>
    constexpr bool is_invocable_v = std::is_invocable_v<Functor, ParamTypes...>;
#else
    template <typename Functor, typename... ParamTypes>
    constexpr bool is_invocable_v = true; // FIXME: implement correctly? mostly only needed to improve diagnostics
#endif

    template <typename R, typename... P>
    using delegate_view_call_t = R (*)(void const* obj, P&&... params);

    template <typename F, typename R, typename... P>
    R delegate_view_thunk(void const* obj, P&&... params) {
        F const& f = *static_cast<F const*>(obj);
        if constexpr (std::is_same_v<R, void>) {
            invoke(f, std::forward<P>(params)...);
        }
        else {
            return invoke(f, std::forward<P>(params)...);
        }
    };

    template <typename ReturnType, typename... ParamTypes>
    class delegate_view_typed {
    protected:
        using call_t = delegate_view_call_t<ReturnType, ParamTypes...>;

    public:
        delegate_view_typed() = delete;

        /// <summary> Construct a new delegate_view from a function object, such as a lambda or function pointer. </summary>
        /// <param name="function"> The function to bind. </param>
        template <typename Functor, typename = enable_if_t<_detail::is_invocable_v<Functor, ParamTypes...> && !std::is_base_of_v<delegate_view_typed, std::decay_t<Functor>>>>
        /*implicit*/ delegate_view_typed(Functor const& functor);

        template <typename Functor, typename = enable_if_t<_detail::is_invocable_v<Functor, ParamTypes...> && !std::is_base_of_v<delegate_view_typed, std::decay_t<Functor>>>>
        delegate_view_typed& operator=(Functor const& functor);

    private:
        template <typename Functor>
        void assign(Functor const& functor);

    protected:
        // we will overwrite this with an object with just a vtable - if we are nullptr, we have no real vtable
        call_t _call = nullptr;
        void const* _functor = nullptr;
    };
} // namespace gm::_detail

/// <summary> Wrapper for a function object, analogous to std::function. </summary>
/// <typeparam name="ReturnType"> Type of the return type. </typeparam>
/// <typeparam name="ParamTypes"> Type of the parameter types. </typeparam>
template <typename ReturnType, typename... ParamTypes>
class gm::delegate_view<ReturnType(ParamTypes...)> : public _detail::delegate_view_typed<ReturnType, ParamTypes...> {
public:
    using _detail::delegate_view_typed<ReturnType, ParamTypes...>::delegate_view_typed;

    /// <summary> Invoke the bound delegate_view, which must not be empty. </summary>
    /// <param name="params"> Function arguments. </param>
    /// <returns> The return value of the bound function, if any. </returns>
    inline ReturnType operator()(ParamTypes... params) const;
};

template <typename ReturnType, typename... ParamTypes>
class gm::delegate_view<ReturnType(ParamTypes...) const> : public _detail::delegate_view_typed<ReturnType, ParamTypes...> {
public:
    using _detail::delegate_view_typed<ReturnType, ParamTypes...>::delegate_view_typed;

    /// <summary> Invoke the bound delegate_view, which must not be empty. </summary>
    /// <param name="params"> Function arguments. </param>
    /// <returns> The return value of the bound function, if any. </returns>
    inline ReturnType operator()(ParamTypes... params) const;
};

template <typename ReturnType, typename... ParamTypes>
template <typename Functor, typename>
gm::_detail::delegate_view_typed<ReturnType, ParamTypes...>::delegate_view_typed(Functor const& functor) {
    assign(functor);
}

template <typename ReturnType, typename... ParamTypes>
template <typename Functor, typename>
auto gm::_detail::delegate_view_typed<ReturnType, ParamTypes...>::operator=(Functor const& functor) -> delegate_view_typed& {
    assign(functor);
    return *this;
}

template <typename ReturnType, typename... ParamTypes>
template <typename Functor>
void gm::_detail::delegate_view_typed<ReturnType, ParamTypes...>::assign(Functor const& functor) {
    using FunctorType = std::decay_t<Functor>;

    this->_call = &_detail::delegate_view_thunk<FunctorType, ReturnType, ParamTypes...>;
    this->_functor = &functor;
}

template <typename ReturnType, typename... ParamTypes>
auto gm::delegate_view<ReturnType(ParamTypes...)>::operator()(ParamTypes... params) const -> ReturnType {
    return this->_call(this->_functor, std::forward<ParamTypes>(params)...);
}

template <typename ReturnType, typename... ParamTypes>
auto gm::delegate_view<ReturnType(ParamTypes...) const>::operator()(ParamTypes... params) const -> ReturnType {
    return this->_call(this->_functor, std::forward<ParamTypes>(params)...);
}
