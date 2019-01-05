#include <iostream>
#include "grimm/foundation/traits.h"

namespace gm {
    template <typename T>
    auto operator<<(std::ostream& os, T const& vec) -> gm::enable_if_t<T::component_length != 0, std::ostream&> {
        os << '{' << vec[0];
        for (int i = 1; i < vec.component_length; ++i) {
            os << ',' << vec[i];
        }
        return os << '}';
    }
} // namespace gm
