// Copyright (C) 2014,2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "int_types.h"
#include "platform.h"

namespace gm {
    /// <summary> A uhash-compatible fnv1-a hasher. </summary>
    class fnv1a {
    public:
        using result_type = uint64;

        inline constexpr void operator()(char const* data, size_t size) noexcept;
        explicit constexpr operator result_type() const noexcept { return _state; }

    private:
        static constexpr uint64 offset = 14695981039346656037ULL;

        result_type _state = offset;
    };

    GM_FORCEINLINE constexpr void fnv1a::operator()(char const* data, size_t size) noexcept {
        constexpr uint64 prime = 1099511628211ULL;

        for (size_t i = 0; i != size; ++i) {
            _state ^= static_cast<uint64>(data[i]);
            _state *= prime;
        }
    }
} // namespace gm
