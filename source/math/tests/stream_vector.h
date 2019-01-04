#include "grimm/math/math_traits.h"
#include <iostream>
#include <type_traits>

namespace gm {
    template <typename T>
    auto operator<<(std::ostream& os, T const& vec) -> std::enable_if_t<is_vector_v<T>, std::ostream&> {
        os << '{' << vec[0];
        for (int i = 1; i < vec.component_length; ++i) {
            os << ',' << vec[i];
        }
        return os << '}';
    }
} // namespace gm
