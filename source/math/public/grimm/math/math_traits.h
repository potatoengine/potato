// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/traits.h"

namespace gm::_detail {
    template <typename T>
    using _has_value_type = typename T::value_type;

    template <int N>
    struct _has_size {
        template <typename T>
        using test = char[T::component_length >= N];
    };
}

namespace gm {
    template <typename T, int MinN = 0>
    constexpr bool is_vector_v = is_detected_v<_detail::_has_value_type, T> && is_detected_v<_detail::_has_size<MinN>::template test, T>;
} // namespace gm::constants
