// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/traits.h"

namespace gm::_detail {
    template <typename T>
    using _has_value_type = typename T::value_type;

    template <int N>
    struct _has_size {
        template <typename T>
        using test = decltype(T::component_length >= N);
    };

    template <typename T>
    using _has_shuffle = decltype(static_cast<T*>(0)->shuffle());
}

namespace gm {
    template <typename T, int MinN>
    constexpr bool is_vector_v = is_detected_v<_detail::_has_value_type, T> && is_detected_v<_detail::_has_size<MinN>::template test, T>;

    template <typename T>
    constexpr bool has_shuffle_v = is_detected_v<_detail::_has_shuffle, T>;
} // namespace gm::constants
