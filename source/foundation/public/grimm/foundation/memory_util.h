// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "traits.h"
#include <utility>

namespace gm {
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
    void move_backwards_n(InputIt first, SizeT count, TypeT* out_first);
} // namespace gm

template <typename InputIt, typename SizeT>
void gm::destruct_n(InputIt first, SizeT count) {
    using type = std::remove_reference_t<decltype(*first)>;
    if (!std::is_trivially_destructible<type>::value)
        for (SizeT i = 0; i != count; ++i, ++first)
            first->~type();
}

template <typename InputIt, typename SizeT, typename TypeT>
void gm::unitialized_copy_n(InputIt first, SizeT count, TypeT* out_first) {
    auto const last = first + count;
    while (first != last)
        new (out_first++) TypeT(*first++);
}

template <typename InputIt, typename SizeT, typename TypeT>
void gm::copy_n(InputIt first, SizeT count, TypeT* out_first) {
    auto const last = first + count;
    while (first != last)
        *out_first++ = *first++;
}

template <typename InputIt, typename SizeT, typename TypeT>
void gm::unitialized_move_n(InputIt first, SizeT count, TypeT* out_first) {
    auto const last = first + count;
    while (first != last)
        new (out_first++) TypeT(std::move(*first++));
}

template <typename InputIt, typename SizeT, typename TypeT>
void gm::move_n(InputIt first, SizeT count, TypeT* out_first) {
    auto const last = first + count;
    while (first != last)
        *out_first++ = std::move(*first++);
}

template <typename InputIt, typename SizeT, typename TypeT>
void gm::move_backwards_n(InputIt first, SizeT count, TypeT* out_last) {
    for (auto in = first + count; in != first;)
        *out_last-- = std::move(*--in);
}
