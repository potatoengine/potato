// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.

#pragma once

#include <cinttypes>

namespace gm
{
    class fnv1a;
    namespace _detail
    {
        template <int> struct fnva1_constants;
    }
}

template <>
struct gm::_detail::fnva1_constants<4>
{
    static const std::uint32_t kOffset = 2166136261U;
    static const std::uint32_t kPrime = 16777619U;
    static const std::uint32_t kShift = 16;
};

template <>
struct gm::_detail::fnva1_constants<8>
{
    static const std::uint64_t kOffset = 14695981039346656037ULL;
    static const std::uint64_t kPrime = 1099511628211ULL;
    static const std::uint32_t kShift = 32;
};

/// <summary> A uhash-compatible fnv1-a hasher. </summary>
class gm::fnv1a
{
public:
    using result_type = size_t;

    inline void operator()(void const* data, size_t count);
    inline explicit operator result_type() const;

private:
    size_t _state = _detail::fnva1_constants<sizeof(result_type)>::kOffset;
};

void gm::fnv1a::operator()(void const* data, size_t count)
{
    char const* bytes = static_cast<char const*>(data);
    for (size_t i = 0; i != count; ++i)
    {
        _state ^= static_cast<size_t>(bytes[i]);
        _state *= _detail::fnva1_constants<sizeof(result_type)>::kPrime;
    }
}

gm::fnv1a::operator result_type() const
{
#if defined(_MSC_VER)
    // Microsoft's implementation does this, so let's be compatible for std::string's sake
    return _state ^ (_state >> _detail::fnva1_constants<sizeof(result_type)>::kShift);
#else
    return _state;
#endif
}
