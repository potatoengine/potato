// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#pragma once

#include "debug.h"
#include "platform.h"
#include "string_format.h"

#include <cstdlib>

#if defined(NDEBUG)

#	define GM_ASSERT(condition, ...) do{}while(false)
#	define GM_UNREACHABLE(...) do{}while(false)

#elif _PREFAST_ // Microsoft's /analyze tool

#	define GM_ASSERT(condition, ...) __analysis_assume(condition)
#	define GM_UNREACHABLE(...) __analysis_assume(false)

#else

namespace gm::_detail
{
	// abstraction to deal with assert instances that don't have a message at all
	template <typename Buffer, typename... Args> void constexpr format_failure(Buffer& buffer, char const* format, Args&&... args) { format_into(buffer, format, std::forward<Args>(args)...); }
	template <typename Buffer> void constexpr format_failure(Buffer&) {}
}

#	define _gm_FORMAT_FAIL(condition_text, ...) \
	do{ \
		::gm::format_fixed_buffer<512> _gm_fail_buffer; \
		_detail::format_failure(_gm_fail_buffer, __VA_ARGS__); \
		_gm_FAIL((condition_text), _gm_fail_buffer.c_str()); \
	}while(false)

#	define GM_ASSERT(condition, ...) \
	do{ \
		if(GM_UNLIKELY(!((condition)))){ \
			_gm_FORMAT_FAIL(#condition, __VA_ARGS__); \
		} \
	}while(false)

#	define GM_UNREACHABLE(...) _gm_FAIL("unreachable code", __VA_ARGS__)

#endif // _PREFAST_
