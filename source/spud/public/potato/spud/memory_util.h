// Copyright (C) 2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "traits.h"
#include <utility>
#include <cstring>

namespace up {
    template <typename InputIt, typename SizeT>
    void destruct_n(InputIt first, SizeT count);

    template <typename InputIt, typename SizeT, typename TypeT>
    void unitialized_copy_n(InputIt first, SizeT count, TypeT* out_first);

    template <typename InputIt, typename SizeT, typename TypeT>
    void copy_n(InputIt first, SizeT count, TypeT* out_first);

    template <typename InputIt, typename SizeT, typename TypeT>
    void unitialized_move_n(InputIt first, SizeT count, TypeT* out_first);

    template <typename InputIt, typename SizeT, typename TypeT>
    void move_n(InputIt first, SizeT count, TypeT* out_first);

    template <typename InputIt, typename SizeT, typename TypeT>
    void move_backwards_n(InputIt first, SizeT count, TypeT* out_last);
} // namespace up

template <typename InputIt, typename SizeT>
void up::destruct_n(InputIt first, SizeT count) {
    using type = std::remove_reference_t<decltype(*first)>;
    if constexpr (!std::is_trivially_destructible_v<type>) {
        if (!std::is_trivially_destructible_v<type>) {
            for (SizeT i = 0; i != count; ++i, ++first) {
                first->~type();
            }
        }
    }
}

template <typename InputIt, typename SizeT, typename TypeT>
void up::unitialized_copy_n(InputIt first, SizeT count, TypeT* out_first) {
    using type = std::remove_reference_t<decltype(*first)>;
    if constexpr (std::is_trivially_constructible_v<TypeT, type> && std::is_pointer_v<InputIt>) {
        std::memmove(out_first, first, count * sizeof(type));
    }
    else {
        auto const last = first + count;
        while (first != last) {
            new (out_first++) TypeT(*first++);
        }
    }
}

template <typename InputIt, typename SizeT, typename TypeT>
void up::copy_n(InputIt first, SizeT count, TypeT* out_first) {
    using type = std::remove_reference_t<decltype(*first)>;
    if constexpr (std::is_trivially_assignable_v<TypeT, type> && std::is_pointer_v<InputIt>) {
        std::memmove(out_first, first, count * sizeof(type));
    }
    else {
        auto const last = first + count;
        while (first != last) {
            *out_first++ = *first++;
        }
    }
}

template <typename InputIt, typename SizeT, typename TypeT>
void up::unitialized_move_n(InputIt first, SizeT count, TypeT* out_first) {
    using type = std::remove_reference_t<decltype(*first)>;
    if constexpr (std::is_trivially_constructible_v<TypeT, type&&> && std::is_pointer_v<InputIt>) {
        std::memmove(out_first, first, count * sizeof(type));
    }
    else {
        auto const last = first + count;
        while (first != last) {
            new (out_first++) TypeT(std::move(*first++));
        }
    }
}

template <typename InputIt, typename SizeT, typename TypeT>
void up::move_n(InputIt first, SizeT count, TypeT* out_first) {
    using type = std::remove_reference_t<decltype(*first)>;
    if constexpr (std::is_trivially_assignable_v<TypeT, type&&> && std::is_pointer_v<InputIt>) {
        std::memmove(out_first, first, count * sizeof(type));
    }
    else {
        auto const last = first + count;
        while (first != last) {
            *out_first++ = std::move(*first++);
        }
    }
}

template <typename InputIt, typename SizeT, typename TypeT>
void up::move_backwards_n(InputIt first, SizeT count, TypeT* out_last) {
    using type = std::remove_reference_t<decltype(*first)>;
    if constexpr (std::is_trivially_assignable_v<TypeT, type&&> && std::is_pointer_v<InputIt>) {
        std::memmove(out_last - count, first - count, count * sizeof(type));
    }
    else {
        for (auto in = first + count; in != first;) {
            *out_last-- = std::move(*--in);
        }
    }
}
