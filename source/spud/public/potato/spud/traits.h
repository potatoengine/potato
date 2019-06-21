// Copyright (C) 2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "typelist.h"
#include <type_traits>
#include <utility>

namespace up {
    namespace _detail {
        template <typename V, template <typename...> class C, typename... A>
        struct detector : std::false_type {};

        template <template <typename...> class C, typename... A>
        struct detector<std::void_t<C<A...>>, C, A...> : std::true_type {};
    } // namespace _detail

    template <typename T>
    struct is_contiguous { static constexpr bool value = std::is_integral_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>; };

    template <typename T>
    constexpr bool is_contiguous_v = is_contiguous<T>::value;

    template <typename F>
    struct signature;
    template <typename S>
    struct function_params;
    template <typename S>
    struct function_result;

    template <typename F>
    using signature_t = typename signature<F>::type;

    template <typename R, typename... A>
    struct function_params<R(A...)> { using type = up::typelist<A...>; };

    template <typename R, typename... A>
    struct function_result<R(A...)> { using type = R; };

    template <template <class...> class Op, class... Args>
    constexpr bool is_detected_v = _detail::detector<void, Op, Args...>::value;

    template <bool C, typename T = void>
    using enable_if_t = typename std::enable_if_t<C, T>;

    template <typename T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(std::is_same_v<int, remove_cvref_t<int const&>>);

    template <typename T>
    constexpr bool is_numeric_v = std::is_integral_v<T> || std::is_floating_point_v<T>;
    static_assert(is_numeric_v<bool> && is_numeric_v<char> && is_numeric_v<float>);
    static_assert(!is_numeric_v<int*> && !is_numeric_v<float&> && !is_numeric_v<std::nullptr_t>);

#if defined(__cpp_lib_constexpr_invoke)
    using std::invoke;
#else
    template <class Class, class Return, class First, class... Rest>
    constexpr decltype(auto) invoke(Return Class::*func, First&& first, Rest&&... rest) {
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

    template <typename T, typename Arguments, typename = void>
    struct is_braces_constructible { static constexpr bool value = false; };

    template <typename T, typename... A>
    struct is_braces_constructible<T, typelist<A...>, std::void_t<decltype(T{std::declval<A>()...})>> { static constexpr bool value = true; };

    template <typename T, typename... A>
    constexpr bool is_braces_constructible_v = is_braces_constructible<T, typelist<A...>>::value;

    struct any_type {
        template <typename T>
        constexpr operator T();
    };
} // namespace up

#if defined(UP_PLATFORM_WINDOWS)
template <typename R, typename... A>
struct up::signature<R __stdcall(A...)> { using type = R(A...); };
template <typename R, typename... A>
struct up::signature<R __vectorcall(A...)> { using type = R(A...); };
template <typename R, typename... A>
struct up::signature<R(__stdcall*)(A...)> { using type = R(A...); };
template <typename R, typename... A>
struct up::signature<R(__vectorcall*)(A...)> { using type = R(A...); };
template <typename T, typename R, typename... A>
struct up::signature<R (__thiscall T::*)(A...)> { using type = R(T&, A...); };
template <typename T, typename R, typename... A>
struct up::signature<R (__vectorcall T::*)(A...)> { using type = R(T&, A...); };
template <typename T, typename R, typename... A>
struct up::signature<R (__thiscall T::*)(A...) const> { using type = R(T const&, A...); };
template <typename T, typename R, typename... A>
struct up::signature<R (__vectorcall T::*)(A...) const> { using type = R(T const&, A...); };
#else
template <typename R, typename... A>
struct up::signature<R(A...)> { using type = R(A...); };
template <typename R, typename... A>
struct up::signature<R (*)(A...)> { using type = R(A...); };
template <typename T, typename R, typename... A>
struct up::signature<R (T::*)(A...)> { using type = R(T&, A...); };
#endif
