#pragma once

#include "grimm/foundation/traits.h"
#include "grimm/math/common.h"
#include <iostream>

namespace gm {
    template <typename T>
    auto operator<<(std::ostream& os, T const& vec) -> gm::enable_if_t<gm::is_vector_v<T>, std::ostream&> {
        os << '{' << vec[0];
        for (int i = 1; i < gm::component_length_v<T>; ++i) {
            os << ',' << vec[i];
        }
        return os << '}';
    }
} // namespace gm
