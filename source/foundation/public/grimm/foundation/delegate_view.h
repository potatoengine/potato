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
    template <typename F, typename R, typename... P>
    R delegate_view_thunk(void const* obj, P&&... params) {
        F const& f = *static_cast<F const*>(obj);
        if constexpr (std::is_void_v<R>) {
            invoke(f, std::forward<P>(params)...);
        }
        else {
            return invoke(f, std::forward<P>(params)...);
        }
    };

    template <typename ReturnType, typename... ParamTypes>
    struct delegate_view_holder {
        using call_t = ReturnType (*)(void const*, ParamTypes&&...);

        delegate_view_holder() = delete;
        template <typename Functor>
        delegate_view_holder(Functor&& functor) {
            using FunctorType = std::decay_t<Functor>;
            _call = &_detail::delegate_view_thunk<FunctorType, ReturnType, ParamTypes...>;
            _functor = &functor;
        }

        call_t _call = nullptr;
        void const* _functor = nullptr;
    };

    template <typename ReturnType, typename... ParamTypes>
    class delegate_view_typed {
    protected:
        using holder_t = delegate_view_holder<ReturnType, ParamTypes...>;

    public:
        /// <summary> Construct a new delegate_view from a function object, such as a lambda or function pointer. </summary>
        /// <param name="function"> The function to bind. </param>
        template <typename Functor, typename = enable_if_t<is_invocable_v<Functor, ParamTypes...> && !std::is_base_of_v<delegate_view_typed, std::decay_t<Functor>>>>
        /*implicit*/ delegate_view_typed(Functor const& functor) : _holder(functor) {}

        template <typename Functor, typename = enable_if_t<is_invocable_v<Functor, ParamTypes...> && !std::is_base_of_v<delegate_view_typed, std::decay_t<Functor>>>>
        delegate_view_typed& operator=(Functor const& functor) {
            _holder = holder_t(functor);
            return *this;
        }

        template <typename... InParamTypes>
        ReturnType operator()(InParamTypes&&... params) const {
            return _holder._call(_holder._functor, std::forward<InParamTypes>(params)...);
        }

    private:
        template <typename Functor>
        void assign(Functor const& functor);

    protected:
        holder_t _holder;
    };
} // namespace gm::_detail

/// <summary> Wrapper for a function object, analogous to std::function. </summary>
/// <typeparam name="ReturnType"> Type of the return type. </typeparam>
/// <typeparam name="ParamTypes"> Type of the parameter types. </typeparam>
template <typename ReturnType, typename... ParamTypes>
class gm::delegate_view<ReturnType(ParamTypes...)> : public _detail::delegate_view_typed<ReturnType, ParamTypes...> {
public:
    using _detail::delegate_view_typed<ReturnType, ParamTypes...>::delegate_view_typed;
};

template <typename ReturnType, typename... ParamTypes>
class gm::delegate_view<ReturnType(ParamTypes...) const> : public _detail::delegate_view_typed<ReturnType, ParamTypes...> {
public:
    using _detail::delegate_view_typed<ReturnType, ParamTypes...>::delegate_view_typed;
};
