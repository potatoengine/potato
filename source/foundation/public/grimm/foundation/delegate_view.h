// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "traits.h"

namespace gm {
    template <typename Signature>
    class delegate_view;

    template <typename T>
    delegate_view(T)->delegate_view<signature_t<T>>;
} // namespace gm

namespace gm::_detail {
    template <typename F, typename R, typename... P>
    R delegate_view_thunk(void const* obj, P&&... params) {
        F const& f = *static_cast<F const*>(obj);
        if constexpr (std::is_void_v<R>) {
            f(std::forward<P>(params)...);
        }
        else {
            return f(std::forward<P>(params)...);
        }
    };

    template <typename ReturnType, typename... ParamTypes>
    struct delegate_view_holder {
        using call_t = ReturnType (*)(void const*, ParamTypes&&...);

        delegate_view_holder() = delete;
        template <typename Functor>
        delegate_view_holder(Functor&& functor) {
            using FunctorType = std::remove_reference_t<Functor>;
            _call = &_detail::delegate_view_thunk<FunctorType, ReturnType, ParamTypes...>;
            _functor = &functor;
        }

        call_t _call = nullptr;
        void const* _functor = nullptr;
    };
} // namespace gm::_detail

template <typename ReturnType, typename... ParamTypes>
class gm::delegate_view<ReturnType(ParamTypes...)> {
private:
    using holder_t = _detail::delegate_view_holder<ReturnType, ParamTypes...>;

    template <typename Functor>
    static constexpr bool is_compatible_v = is_invocable_v<Functor, ParamTypes...> && !std::is_member_function_pointer_v<Functor> && !std::is_base_of_v<delegate_view, std::decay_t<Functor>>;

public:
    delegate_view() = delete;
    delegate_view(delegate_view const&) = default;
    delegate_view& operator=(delegate_view const&) = default;

    template <typename Functor, typename = enable_if_t<is_compatible_v<Functor>>>
    /*implicit*/ delegate_view(Functor&& functor) : _holder(std::forward<Functor>(functor)) {}

    template <typename Functor, typename = enable_if_t<is_compatible_v<Functor>>>
    delegate_view& operator=(Functor&& functor) {
        _holder = holder_t(std::forward<Functor>(functor));
        return *this;
    }

    template <typename... InParamTypes>
    ReturnType operator()(InParamTypes&&... params) const {
        return _holder._call(_holder._functor, std::forward<InParamTypes>(params)...);
    }

private:
    template <typename Functor>
    void assign(Functor const& functor);

    holder_t _holder;
};
