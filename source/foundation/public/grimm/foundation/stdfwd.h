// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

namespace std
{
	template <typename> class allocator;

	template <typename> struct char_traits;

	template <typename, typename, typename> class basic_string;
	template <typename, typename> class vector;

	using string = basic_string<char, char_traits<char>, allocator<char>>;
}