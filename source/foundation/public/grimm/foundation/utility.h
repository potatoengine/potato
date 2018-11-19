// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm
{
	/// Convenient base class or tag.
	struct empty {};

	/// N3911 - http://isocpp.org/files/papers/N3911.pdf
	template <typename...>
	using void_t = void;
	
	/// Take any number of parameters and do nothing with them.
	/// Useful for some template metaprogramming tasks.
	template <typename... T>
	constexpr void ignore(T&&...) {}

	// passthru functor
	struct identity
	{
		template <typename T>
		constexpr auto operator()(T&& value) const -> decltype(auto) { return std::forward<T>(value); }
	};

	template <typename R>
	constexpr auto begin(R&& r) -> decltype(r.begin())
	{
		return r.begin();
	}

	template <typename T, int N>
	constexpr T* begin(T(&a)[N])
	{
		return a;
	}

	template <typename R>
	constexpr auto end(R&& r) -> decltype(r.end())
	{
		return r.end();
	}

	template <typename T, int N>
	constexpr T* end(T(&a)[N])
	{
		return a + N;
	}

	template <typename R>
	constexpr auto size(R&& r) -> decltype(r.size())
	{
		return r.size();
	}

	template <typename T, int N>
	constexpr int size(T(&a)[N])
	{
		return N;
	}

	template <typename R>
	constexpr auto data(R&& r) -> decltype(r.data())
	{
		return r.data();
	}

	template <typename T, int N>
	constexpr T* data(T(&a)[N])
	{
		return a;
	}

	template <typename T>
	constexpr T&& move(T& v)
	{
		return static_cast<T&&>(v);
	}
}
