// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

// Based on Howard Hinnant's "Types Don't Know #"

#pragma once

#include "fnv1a.h"
#include "traits.h"
#include <string>
#include <type_traits>
#include <typeindex>
#include <utility>

namespace gm {
    template <typename HashAlgorithm>
    struct uhash;

    template <typename, typename>
    class vector;
} // namespace gm

namespace gm {
    template <typename HashAlgorithm, typename T>
    inline std::enable_if_t<is_contiguous<T>::value> hash_append(HashAlgorithm& hasher, T const& value) {
        hasher(&value, sizeof(value));
    }

    template <typename HashAlgorithm, typename CharT, typename CharTraits, typename AllocatorT>
    inline void hash_append(HashAlgorithm& hasher, std::basic_string<CharT, CharTraits, AllocatorT> const& string) {
        hasher(string.c_str(), string.size());
    }

    template <typename HashAlgorithm, typename ContainerT, typename, typename>
    inline void hash_append(HashAlgorithm& hasher, ContainerT const& container) {
        for (auto const& value : container)
            hash_append(hasher, value);
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

    template <typename HashAlgorithm, typename ValueT, typename AllocatorT>
    inline std::enable_if_t<is_contiguous<ValueT>::value> hash_append(HashAlgorithm& hasher, vector<ValueT, AllocatorT> const& container) {
        hasher(container.data(), container.size() * sizeof(ValueT));
    }

    template <typename HashAlgorithm, typename ValueT, typename AllocatorT>
    inline std::enable_if_t<!is_contiguous<ValueT>::value> hash_append(HashAlgorithm& hasher, vector<ValueT, AllocatorT> const& container) {
        for (auto&& value : container)
            hash_append(hasher, value);
    }
} // namespace gm

template <typename HashAlgorithm = gm::fnv1a>
struct uhash {
    using result_type = typename HashAlgorithm::result_type;

    template <typename T>
    result_type operator()(T&& value) const {
        HashAlgorithm hasher;
        using gm::hash_append;
        hash_append(hasher, value);
        return static_cast<result_type>(hasher);
    }
};
