// Copyright (C) 2014,2019 Sean Middleditch, all rights reserverd.

// Based on Howard Hinnant's "Types Don't Know #"

#pragma once

#include "hash_fnv1a.h"
#include "traits.h"
#include "int_types.h"
#include <string>
#include <type_traits>
#include <typeindex>
#include <utility>

namespace gm {
    struct default_hash;

    template <typename HashAlgorithm = default_hash>
    struct uhash;

    template <typename HashAlgorithm = default_hash, typename T>
    auto hash_value(T const& value) -> typename HashAlgorithm::result_type;

    template <typename>
    class vector;
} // namespace gm

namespace gm {
    template <typename HashAlgorithm, typename T>
    inline enable_if_t<is_contiguous<T>::value> hash_append(HashAlgorithm& hasher, T const& value) {
        hasher.append_bytes(&value, sizeof(value));
    }

    template <typename HashAlgorithm, typename CharT, typename CharTraits, typename AllocatorT>
    inline void hash_append(HashAlgorithm& hasher, std::basic_string<CharT, CharTraits, AllocatorT> const& string) {
        hasher.append_bytes(string.data(), string.size());
    }

    template <typename HashAlgorithm, typename ContainerT, typename, typename>
    inline void hash_append(HashAlgorithm& hasher, ContainerT const& container) {
        for (auto const& value : container) {
            hash_append(hasher, value);
        }
    }

    template <typename HashAlgorithm, typename FirstT, typename SecondT>
    inline void hash_append(HashAlgorithm& hasher, std::pair<FirstT, SecondT> const& pair) {
        hash_append(hasher, pair.first);
        hash_append(hasher, pair.second);
    }

    template <typename HashAlgorithm>
    inline void hash_append(HashAlgorithm& hasher, std::type_index type) {
        // hash_code might not be the same number of bits as the hasher so we rehash.
        // we rehash even if they are the same number of bits, which is probably not ideal, but works.
        hash_append(hasher, type.hash_code());
    }

    // note: [[1, 2], 3] will hash the same as [1, [2, 3]]
    //       likewise, ["a", "bc"] will hash the same as ["ab", "c"]

    template <typename HashAlgorithm, typename ValueT>
    inline enable_if_t<is_contiguous<ValueT>::value> hash_append(HashAlgorithm& hasher, vector<ValueT> const& container) {
        hasher({container.data(), container.size() * sizeof(ValueT)});
    }

    template <typename HashAlgorithm, typename ValueT, typename AllocatorT>
    inline enable_if_t<!is_contiguous<ValueT>::value> hash_append(HashAlgorithm& hasher, vector<ValueT> const& container) {
        for (auto&& value : container) {
            hash_append(hasher, value);
        }
    }
} // namespace gm

struct gm::default_hash {
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
struct gm::uhash {
    using result_type = typename HashAlgorithm::result_type;

    template <typename T>
    result_type operator()(T&& value) const {
        return hash_value(value);
    }
};

template <typename HashAlgorithm, typename T>
auto gm::hash_value(T const& value) -> typename HashAlgorithm::result_type {
    HashAlgorithm hasher{};
    using gm::hash_append;
    hash_append(hasher, value);
    return hasher.finalize();
}
