// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "typelist.h"
#include <type_traits>

namespace gm
{
	template <typename T> struct is_range;
	template <typename T> struct is_contiguous;
	template <typename F> struct signature;
	template <typename S> struct function_params;
	template <typename S> struct function_result;

	template <typename T> constexpr bool is_range_v = is_range<T>::value;
	template <typename T> constexpr bool is_contiguous_v = is_contiguous<T>::value;
	template <typename F> using signature_t = typename signature<F>::type;
	template <typename S> using function_params_t = typename function_params<S>::type;
	template <typename S> using function_result_t = typename function_result<S>::type;
}

template <typename T> struct gm::is_range : std::false_type {};

template <typename T> struct gm::is_contiguous : std::integral_constant<bool, std::is_integral_v<T> || std::is_enum_v<T> || std::is_pointer_v<T>> {};

template <typename R, typename... A> struct gm::signature<R(__stdcall)(A...)> { using type = R(A...); };
template <typename R, typename... A> struct gm::signature<R(__vectorcall)(A...)> { using type = R(A...); };
template <typename R, typename... A> struct gm::signature<R(__stdcall *)(A...)> { using type = R(A...); };
template <typename R, typename... A> struct gm::signature<R(__vectorcall *)(A...)> { using type = R(A...); };
template <typename T, typename R, typename... A> struct gm::signature<R(__thiscall T::*)(A...)> { using type = R(T&, A...); };
template <typename T, typename R, typename... A> struct gm::signature<R(__vectorcall T::*)(A...)> { using type = R(T&, A...); };
template <typename T, typename R, typename... A> struct gm::signature<R(__thiscall T::*)(A...) const> { using type = R(T const&, A...); };
template <typename T, typename R, typename... A> struct gm::signature<R(__vectorcall T::*)(A...) const> { using type = R(T const&, A...); };

template <typename R, typename... A> struct gm::function_params<R(A...)> { using type = gm::typelist<A...>; };
template <typename R, typename... A> struct gm::function_result<R(A...)> { using type = R; };
