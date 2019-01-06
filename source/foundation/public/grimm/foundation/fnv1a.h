// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "types.h"
#include "span.h"
#include "platform.h"

namespace gm {
    /// <summary> A uhash-compatible fnv1-a hasher. </summary>
    class fnv1a {
    public:
        using result_type = uint64;

        inline void operator()(span<byte const> data) noexcept;
        explicit operator result_type() const noexcept { return _state; }

    private:
        static constexpr uint64 offset = 14695981039346656037ULL;

        result_type _state = offset;
    };

    GM_FORCEINLINE void fnv1a::operator()(span<byte const> data) noexcept {
        constexpr uint64 prime = 1099511628211ULL;

        for (std::byte c : data) {
            _state ^= static_cast<uint64>(c);
            _state *= prime;
        }
    }
} // namespace gm
