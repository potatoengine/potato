// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "typelist.h"
#include <type_traits>
#include <utility>

namespace gm::_detail {
    template <typename V, template <typename...> class C, typename... A>
    struct detector : std::false_type {};

    template <template <typename...> class C, typename... A>
    struct detector<std::void_t<C<A...>>, C, A...> : std::true_type {};
} // namespace gm::_detail

namespace gm {
    template <typename T>
    struct is_range : std::false_type {};

    template <typename T>
    struct is_contiguous : std::integral_constant<bool, std::is_integral_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>> {};

    template <typename F>
    struct signature;
    template <typename S>
    struct function_params;
    template <typename S>
    struct function_result;

    template <typename T>
    constexpr bool is_range_v = is_range<T>::value;
    template <typename T>
    constexpr bool is_contiguous_v = is_contiguous<T>::value;
    template <typename F>
    using signature_t = typename signature<F>::type;

    template <typename R, typename... A>
    struct function_params<R(A...)> { using type = gm::typelist<A...>; };

    template <typename R, typename... A>
    struct function_result<R(A...)> { using type = R; };

    template <template <class...> class Op, class... Args>
    constexpr bool is_detected_v = _detail::detector<void, Op, Args...>::value;

    template <bool C, typename T = void>
    using enable_if_t = typename std::enable_if_t<C, T>;

#if defined(__cpp_lib_invoke)
    using std::invoke;
#else
    template <class Class, class Return, class First, class... Rest>
    decltype(auto) invoke(Return Class::*func, First&& first, Rest&&... rest) {
        if constexpr (std::is_member_function_pointer_v<decltype(func)>) {
            return (std::forward<First>(first).*func)(std::forward<Rest>(rest)...);
        }
        else {
            return func(std::forward<First>(first), std::forward<Rest>(rest)...);
        }
    }

    template <typename Functor, typename... Args>
    constexpr auto invoke(Functor&& func, Args&&... args) -> decltype(std::forward<Functor>(func)(std::forward<Args>(args)...)) {
        return std::forward<Functor>(func)(std::forward<Args>(args)...);
    }
#endif

#if defined(__cpp_lib_is_invocable)
    template <typename Functor, typename... ParamTypes>
    constexpr bool is_invocable_v = std::is_invocable_v<Functor, ParamTypes...>;
#else
    namespace _detail {
        template <typename Functor, typename... ParamTypes>
        using is_invocable_test = decltype(invoke(std::declval<Functor>(), std::declval<ParamTypes>()...));
        extern void is_invocable_func_test();
    } // namespace _detail
    template <typename Functor, typename... ParamTypes>
    constexpr bool is_invocable_v = is_detected_v<_detail::is_invocable_test, Functor, ParamTypes...>;
    static_assert(is_invocable_v<decltype(_detail::is_invocable_func_test)>);
    static_assert(!is_invocable_v<int>);
#endif
} // namespace gm

#if defined(GM_PLATFORM_WINDOWS)
template <typename R, typename... A>
struct gm::signature<R __stdcall(A...)> { using type = R(A...); };
template <typename R, typename... A>
struct gm::signature<R __vectorcall(A...)> { using type = R(A...); };
template <typename R, typename... A>
struct gm::signature<R(__stdcall*)(A...)> { using type = R(A...); };
template <typename R, typename... A>
struct gm::signature<R(__vectorcall*)(A...)> { using type = R(A...); };
template <typename T, typename R, typename... A>
struct gm::signature<R (__thiscall T::*)(A...)> { using type = R(T&, A...); };
template <typename T, typename R, typename... A>
struct gm::signature<R (__vectorcall T::*)(A...)> { using type = R(T&, A...); };
template <typename T, typename R, typename... A>
struct gm::signature<R (__thiscall T::*)(A...) const> { using type = R(T const&, A...); };
template <typename T, typename R, typename... A>
struct gm::signature<R (__vectorcall T::*)(A...) const> { using type = R(T const&, A...); };
#else
template <typename R, typename... A>
struct gm::signature<R(A...)> { using type = R(A...); };
template <typename R, typename... A>
struct gm::signature<R (*)(A...)> { using type = R(A...); };
template <typename T, typename R, typename... A>
struct gm::signature<R (T::*)(A...)> { using type = R(T&, A...); };
#endif
