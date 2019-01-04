#include <iostream>
#include "grimm/math/math_traits.h"

namespace gm {
    template <typename T, typename = std::enable_if_t<is_vector_v<T>>>
    std::ostream& operator<<(std::ostream& os, T const& vec) {
        os << '{' << vec[0];
        for (int i = 1; i < vec.component_length; ++i) {
            os << ',' << vec[i];
        }
        return os << '}';
    }
} // namespace gm
