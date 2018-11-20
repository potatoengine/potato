// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#pragma once

// detect platform
#if defined(_WIN32) || defined(_WINDOWS)
#	define GM_PLATFORM_WINDOWS 1
#	if defined(_WIN64)
#		define GM_PLATFORM_WIN64 1
#	else
#		define GM_PLATFORM_WIN32 1
#	endif
#elif __APPLE__
#	define GM_PLATFORM_APPLE 1
#	define GM_PLATFORM_POSIX 1
#	include "TargetConditionals.h"
#	if TARGET_OS_IPHONE
#		define GM_PLATFORM_IOS 1
#	else
#		define GM_PLATFORM_OSX 1
#	endif
#elif __linux
#	define GM_PLATFORM_LINUX 1
#	define GM_PLATFORM_POSIX 1
#	if defined(__ANDROID__)
#		define GM_PLATFORM_ANDROID 1
#	endif
#elif __posix
#	define GM_PLATFORM_POSIX 1
#else
#	error "Unsupported platform"
#endif

// detect build configurations
#if defined(_DEBUG) || !defined(NDEBUG)
#	define GM_BUILD_DEBUG 1
#else
#	define GM_BUILD_RELEASE 1
#endif

// detect architecture and details
// #FIXME: this is very Windows-only right now and doesn't handle ARM or HTML5
#if defined(_M_X64) || defined(__x86_64__)
#	define GM_ARCH_LITTLE_ENDIAN 1
#	define GM_ARCH_INTEL 1
#	define GM_ARCH_64 1
#	define GM_ARCH_LLP64 1
#	define GM_ARCH_CACHELINE 64
#else
#	error "Unsupported architecture"
#endif

// ensure GM_API is defined
#if !defined(GM_API)
//#   error "GM_API must be defined for the compilation mode"
#   define GM_API
#endif

// ensure C++17
// #if __cplusplus < 201703L
// #   error "C++17 or higher is required"
// #endif

// compiler detection
#if defined(_MSC_VER)
#	if _MSC_VER >= 1910
#		define GM_COMPILER_MICROSOFT 1
#		define GM_COMPILER_VERSION (_MSC_VER / 100)
#		define GM_THREAD_LOCAL __declspec(thread)
#	else
#		error "Unsupported Visual C++ compiler version (requires 19.10 or higher from Visual Studio 2017)"
#	endif
#elif defined(__clang__)
#	if __clang_major__ > 7
#		define GM_COMPILER_CLANG 1
#		define GM_COMPILER_VERSION (__clang_major__ * 10 + __clang_minor__)
#		define GM_THREAD_LOCAL thread_local
#	else
#		error "Unsupported Clang (requires 3.5 or newer)"
#	endif
#elif defined(__GNUC__)
#	if __GNUC__ > 5 || (__GNUC__ == 5 && __GNUC_MINOR__ >= 2)
#		define GM_COMPILER_GCC 1
#		define GM_COMPILER_VERSION (__GNUC__ * 10 + __GNUC_MINOR__)
#		define GM_THREAD_LOCAL __thread
#	else
#		error "Unsupported GCC (requires 5.2 or newer)"
#	endif
#else
#	error "Unsupported compiler"
#endif

// setup calling conventions, intrinsics, etc.
#if GM_PLATFORM_WINDOWS
#	define GM_STDCALL __stdcall
#	define GM_VECTORCALL __vectorcall
#else
#	define GM_STDCALL
#	define GM_VECTORCALL
#endif

#if GM_COMPILER_MICROSOFT
#	define GM_NOINLINE __declspec(noinline)
#	define GM_NORETURN __declspec(noreturn)
#	define GM_FORCEINLINE __forceinline
#	define GM_LIKELY(x) (x)
#	define GM_UNLIKELY(x) (x)
#	define GM_ASSUME(x) __assume((x))
#elif GM_COMPILER_CLANG || GM_COMPILER_GCC
#	define GM_NOINLINE [[noinline]]
#	define GM_NORETURN [[noreturn]]
#	define GM_FORCEINLINE [[flatten]]
#	define GM_LIKELY(x) __builtin_expect((x), 1)
#	define GM_UNLIKELY(x) __builtin_expect((x), 0)
#	define GM_ASSUME(x) __builtin_assume((x))
#endif
