// Copyright (C) 2014,2019 Sean Middleditch, all rights reserverd.

// Based on Howard Hinnant's "Types Don't Know #"

#pragma once

#include "hash_fnv1a.h"
#include "traits.h"
#include "int_types.h"

namespace up {
    struct default_hash;

    template <typename HashAlgorithm = default_hash>
    struct uhash;

    template <typename HashAlgorithm = default_hash, typename T>
    auto hash_value(T const& value) -> typename HashAlgorithm::result_type;

    template <class Hash>
    constexpr auto hash_combine(Hash left, Hash right) noexcept -> Hash;

    template <typename>
    class vector;
} // namespace up

namespace up {
    template <typename HashAlgorithm, typename T>
    inline enable_if_t<is_contiguous<T>::value, HashAlgorithm&> hash_append(HashAlgorithm& hasher, T const& value) {
        hasher.append_bytes(reinterpret_cast<char const*>(&value), sizeof(value));
        return hasher;
    }
} // namespace up

struct up::default_hash {
    using result_type = typename fnv1a::result_type;

    constexpr void append_bytes(char const* data, size_t size) noexcept {
        _fnva1.append_bytes(data, size);
    }

    constexpr auto finalize() const noexcept {
        auto result = _fnva1.finalize();
#if defined(_MSC_VER)
        // Microsoft's std::hash implementation does this, so let's be compatible
        // TODO: what does libc++ do?
        // TODO: should this all just forward to std::hash instead of our own hasher?
        result ^= (result >> (4 * sizeof(void*)));
#endif
        return result;
    }

private:
    fnv1a _fnva1;
};

template <typename HashAlgorithm>
struct up::uhash {
    using result_type = typename HashAlgorithm::result_type;

    template <typename T>
    result_type operator()(T&& value) const {
        return hash_value(value);
    }
};

template <typename HashAlgorithm, typename T>
auto up::hash_value(T const& value) -> typename HashAlgorithm::result_type {
    HashAlgorithm hasher{};
    using up::hash_append;
    hash_append(hasher, value);
    return hasher.finalize();
}

template <class Hash>
constexpr auto up::hash_combine(Hash left, Hash right) noexcept -> Hash {
    left ^= right + 0x9e3779b9 + (left << 6) + (left >> 2);
    return left;
}
